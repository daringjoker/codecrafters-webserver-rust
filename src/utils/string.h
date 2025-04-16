#pragma once

#include <string>
#include <vector>

inline bool starts_with(const std::string &str, const std::string &prefix) {
  return str.rfind(prefix, 0) == 0;
}

inline std::string strip_prefix(std::string str, const std::string &prefix) {
  if (starts_with(str, prefix)) {
    str.erase(0, prefix.length());
  }
  return str;
}

inline std::vector<std::string> split(const std::string &str,
                                      const std::string &delim) {
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
