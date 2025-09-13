#pragma once

#include "protocol.hpp"
#include <string>

namespace byoredis {

template <typename Derived>
class RedisHandler {
  Response get(std::string key) {
    return static_cast<Derived*>(this)->get_impl(key);
  }

  Response set(std::string key, std::string value) {
    return static_cast<Derived*>(this)->set_impl(key, value);
  }

  Response del(std::string key) {
    return static_cast<Derived*>(this)->del_impl(key);
  }
};

class IRedisHandler {
public:
  virtual Response get(std::string key) = 0;
  virtual Response set(std::string key, std::string value) = 0;
  virtual Response del(std::string key) = 0;

  virtual ~IRedisHandler() = default;
};

} // namespace byoredis
