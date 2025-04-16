#pragma  once 
#include <map>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <vector>

enum class Method {
  GET,
  POST,
  PUT,
  DELETE,
  PATCH,
  OPTIONS,
  HEAD,
  TRACE,
  CONNECT,
};



std::vector<std::string> split(const std::string &str, const std::string &delim) {
  std::vector<std::string> tokens;
  size_t start = 0;
  size_t end = str.find(delim);
  while (end != std::string::npos) {
    tokens.push_back(str.substr(start, end - start));
    start = end + delim.length();
    end = str.find(delim, start);
  }
  tokens.push_back(str.substr(start, end));
  return tokens;
}


class Request {
private:
  Method method;
  std::string url;
  // std::string body;
  // std::map<std::string, std::string> headers;

  Method parse_method(std::string method_str) {
    auto method_map= std::map<std::string, Method>{
      {"GET", Method::GET},
      {"POST", Method::POST},
      {"PUT", Method::PUT},
      {"DELETE", Method::DELETE},
      {"PATCH", Method::PATCH},
      {"OPTIONS", Method::OPTIONS},
      {"HEAD", Method::HEAD},
      {"TRACE", Method::TRACE},
      {"CONNECT", Method::CONNECT}
    };
    auto it = method_map.find(method_str);
    if (it != method_map.end()) {
      return it->second;
    } else {
      throw std::invalid_argument("Invalid HTTP method");
    }
  }
  
  void parse_request(std::string reqBuffer ){
    std::vector<std::string> lines = split(reqBuffer, "\r\n");
    std::vector<std::string> request_line = split(lines[0], " ");
    if (request_line.size() < 3) {
      throw std::invalid_argument("Invalid request line");
    }
    this->method=parse_method(request_line[0]);
    this->url = request_line[1];
    url = request_line[1];

  }

public:
  Request(int client_fd){
    unsigned long long reqBufSize= 1024*1024;
    char reqBuffer[reqBufSize];
    recv(client_fd, reqBuffer,reqBufSize,0);
    std::string reqBufferStr(reqBuffer);
    parse_request(reqBufferStr);
  }

  Method get_method() const {
    return method;
  }

  std::string get_url() const {
    return url;
  }
};
