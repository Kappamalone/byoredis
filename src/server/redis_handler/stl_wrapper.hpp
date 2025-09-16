#pragma once

#include "../protocol.hpp"
#include "../redis_handler.hpp"
#include <string>
#include <unordered_map>

namespace byoredis {

// simple wrapper around std::unordered_map
class STLWrapper : public RedisHandler<STLWrapper> {
public:
  STLWrapper();
  ~STLWrapper() = default;

  Response get_impl(std::string key);
  Response set_impl(std::string key, std::string value);
  Response del_impl(std::string key);

private:
  std::unordered_map<std::string, std::string> db;
};

} // namespace byoredis
