#pragma once

#include "request.h"
#include "response.h"
#include <vector>

using NextFunction = std::function<void(std::optional<std::string> error)>;
using Middleware = std::function<void(Request &, Response &, NextFunction)>;
using ErrorMiddleware = std::function<void(const std::string &, Request &,
                                           Response &, NextFunction)>;
using Controller = std::function<void(Request &, Response &)>;

class App {
private:
  int server_fd;
  std::vector<Middleware> middlewares;
  std::vector<ErrorMiddleware> error_middlewares;
  void handle_request(int client_fd, const char *base_path);

public:
  App();

  void use();
  void listen(int port);
};
