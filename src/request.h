#pragma once
#include "utils/string.h"
#include <cctype>
#include <iostream>
#include <map>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <vector>

class Request {
public:
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

private:
  Method method;
  std::string url;
  std::string body;
  std::map<std::string, std::string> headers;

  Method parse_method(std::string method_str) {
    auto method_map = std::map<std::string, Method>{
        {"GET", Method::GET},        {"POST", Method::POST},
        {"PUT", Method::PUT},        {"DELETE", Method::DELETE},
        {"PATCH", Method::PATCH},    {"OPTIONS", Method::OPTIONS},
        {"HEAD", Method::HEAD},      {"TRACE", Method::TRACE},
        {"CONNECT", Method::CONNECT}};
    auto it = method_map.find(method_str);
    if (it != method_map.end()) {
      return it->second;
    } else {
      throw std::invalid_argument("Invalid HTTP method");
    }
  }

  void parse_headers(std::vector<std::string> lines) {
    for (size_t i = 1; i < lines.size(); ++i) {
      if (lines[i].empty()) {
        // Skip after empty line
        break;
      }
      auto sepIndex = lines[i].find_first_of(": ");
      if (sepIndex == std::string::npos) {
        throw std::invalid_argument("Invalid header format");
      }
      std::string key = lines[i].substr(0, sepIndex);
      std::transform(key.begin(), key.end(), key.begin(),
                     [](unsigned char c) { return std::tolower(c); });
      std::string value = lines[i].substr(sepIndex + 2);
      if (key.empty() || value.empty()) {
        throw std::invalid_argument("Invalid header format");
      }
      headers[key] = value;
    }
  }

  void parse_request(std::string reqBuffer) {
    std::vector<std::string> lines = split(reqBuffer, "\r\n");
    std::vector<std::string> request_line = split(lines[0], " ");
    if (request_line.size() < 3) {
      throw std::invalid_argument("Invalid request line");
    }
    this->method = parse_method(request_line[0]);
    this->url = request_line[1];
    url = request_line[1];
    parse_headers(lines);
    auto content_length_hdr = this->get_header("content-length");
    auto content_length = std::stoi(content_length_hdr.value_or("0"));

    if (content_length > 0) {
      this->body =
          reqBuffer.substr(reqBuffer.find("\r\n\r\n") + 4, content_length);
    }
  }

public:
  Request(int client_fd) {
    unsigned long long reqBufSize = 1024 * 1024;
    char reqBuffer[reqBufSize];
    recv(client_fd, reqBuffer, reqBufSize, 0);
    std::string reqBufferStr(reqBuffer);
    parse_request(reqBufferStr);
  }

  std::map<std::string, std::string>
  parse_url_parameters(std::string url_mask) {
    std::smatch mask_match;
    std::regex mask_regex("\\{(.*?)\\}");
    std::vector<std::string> param_names;
    if (std::regex_search(url_mask, mask_match, mask_regex)) {
      for (size_t i = 1; i < mask_match.size(); ++i) {
        std::string param_name = mask_match[i].str();
        param_names.push_back(param_name);
      }
    }
    auto finder = std::regex_replace(url_mask, mask_regex, "([^/]+)");
    finder = "^" + finder + "$";

    std::regex url_regex(finder);
    std::smatch url_match;
    std::map<std::string, std::string> parameters;
    if (std::regex_search(this->url, url_match, url_regex)) {
      for (size_t i = 1; i < url_match.size(); ++i) {
        std::string param_name = param_names[i - 1];
        std::string param_value = url_match[i].str();
        parameters[param_name] = param_value;
      }
    }
    return parameters;
  }

  Method get_method() const { return method; }

  std::string get_method_name() const {
    std::map<Method, std::string> method_map = {
        {Method::GET, "GET"},        {Method::POST, "POST"},
        {Method::PUT, "PUT"},        {Method::DELETE, "DELETE"},
        {Method::PATCH, "PATCH"},    {Method::OPTIONS, "OPTIONS"},
        {Method::HEAD, "HEAD"},      {Method::TRACE, "TRACE"},
        {Method::CONNECT, "CONNECT"}};
    return method_map[method];
  }

  std::string get_url() const { return url; }

  std::string get_body() const { return body; }

  std::optional<std::string> get_header(const std::string &key) const {
    auto it = headers.find(key);
    if (it != headers.end()) {
      return it->second;
    } else {
      return std::nullopt;
    }
  }
};
