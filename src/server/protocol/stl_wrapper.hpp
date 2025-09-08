#pragma once

#include "../protocol.hpp"
#include <string>
#include <unordered_map>

namespace byoredis {

// simple wrapper around std::unordered_map
class STLWrapper : public IProtocol {
public:
  STLWrapper();
  ~STLWrapper() = default;

  Response get(std::string key) override;
  Response set(std::string key, std::string value) override;
  Response del(std::string key) override;

private:
  std::unordered_map<std::string, std::string> db;
};

} // namespace byoredis
