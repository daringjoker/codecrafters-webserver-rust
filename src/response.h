#pragma  once
#include <sstream>
#include <string>
#include <sys/socket.h>

class Response {
private:
    int statusCode;
    std::string statusMessage;
    std::string body;

  std::string  buildResponse() {
    std::stringstream responseStream;
    responseStream << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    responseStream << "Content-Length: " << body.length() << "\r\n";
    responseStream << "Content-Type: text/html\r\n";
    responseStream << "Connection: close\r\n";
    responseStream << "\r\n"; // End of headers
    // Add the body
    responseStream << body;
    return responseStream.str();
  }
public:
  Response& setStatusCode(int code) {
    statusCode = code;
    return *this;
  }

  Response& setStatusMessage(const std::string& message) {
    statusMessage = message;
    return *this;
  }

  Response& setBody(const std::string& responseBody) {
    body = responseBody;
    return *this;
  }


  void respond(int client_fd){
    std::string response = buildResponse();
    send(client_fd, response.c_str(), response.length(), 0);
  }
};
