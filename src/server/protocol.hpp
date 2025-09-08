#pragma once

#include <string>

namespace byoredis {

enum class Command {
  GET = 1,
  SET = 2,
  DELETE = 3,
};

struct Response {
  // ...too lazy to specify any sort of status code
  std::string msg;
};

class IProtocol {
public:
  virtual Response get(std::string key) = 0;
  virtual Response set(std::string key, std::string value) = 0;
  virtual Response del(std::string key) = 0;

  virtual ~IProtocol() = default;
};

} // namespace byoredis
