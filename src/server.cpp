#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "request.h"
#include "response.h"
#include "utils/string.h"

void handle_request(int client_fd, std::string base_path);

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  std::string base_path = "";

  if (argc == 3 && argv[1] == std::string("--directory")) {
    base_path = std::string(argv[2]);
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  std::cout << "Waiting for a client to connect...\n";
  std::vector<std::thread> threads;
  while (true) {
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                           (socklen_t *)&client_addr_len);
    threads.emplace_back(handle_request, client_fd, base_path);
    std::cout << "Client connected\n";
  }
  close(server_fd);
  return 0;
}

void serve_file(std::string base_path, std::string file_path, int client_fd) {
  std::cout << "Serving file: " << base_path + file_path << "\n";
  std::ifstream file(base_path + file_path);
  std::cout << "File opened: " << file.is_open() << "\n";
  if (!file.is_open()) {
    Response res;
    res.setStatusCode(404).respond(client_fd);
    close(client_fd);
    return;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string content = buffer.str();
  std::cout << "File content length: " << content.length() << "\n";
  Response res;

  res.setStatusCode(200)
      .setMimeType("application/octet-stream")
      .setBody(content)
      .respond(client_fd);
  std::cout << "File served\n";
}

void write_file(std::string base_path, std::string file_path,
                std::string content, int client_fd) {
  std::ofstream file(base_path + file_path);

  if (!file.is_open()) {
    Response res;
    res.setStatusCode(500).respond(client_fd);
    close(client_fd);
    return;
  }

  file << content;
  file.close();
  Response res;
  res.setStatusCode(201).respond(client_fd);
}

void handle_request(int client_fd, std::string base_path) {

  Request req(client_fd);

  auto url = req.get_url();

  std::cout << "Inbound: [" << req.get_method_name() << "] " << url << "\n";

  if (url == "/") {
    Response res;
    res.setStatusCode(200).respond(client_fd);
    close(client_fd);
    return;
  }

  if (url == "/user-agent") {
    auto agent = req.get_header("user-agent");
    Response res;
    if (agent.has_value()) {
      res.setStatusCode(200).setBody(agent.value()).respond(client_fd);
    } else {
      res.setStatusCode(400).respond(client_fd);
    }
    close(client_fd);
    return;
  }

  bool is_file_route = starts_with(url, "/files/");
  std::cout << "Is file route: " << is_file_route << "\n";

  if (is_file_route) {
    auto file_path = strip_prefix(url, "/files");
    if (req.get_method() == Request::Method::POST) {
      write_file(base_path, file_path, req.get_body(), client_fd);
      close(client_fd);
      return;
    }
    if (req.get_method() == Request::Method::GET) {
      serve_file(base_path, file_path, client_fd);
      close(client_fd);
      return;
    }
  }

  auto url_parameters = req.parse_url_parameters("/echo/{str}");
  auto str = url_parameters.find("str");
  if (str != url_parameters.end()) {
    Response res;
    res.setStatusCode(200).setBody(str->second).respond(client_fd);
  } else {
    Response res;
    res.setStatusCode(404).respond(client_fd);
  }

  close(client_fd);
}
