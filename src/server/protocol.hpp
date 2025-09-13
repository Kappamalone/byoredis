#pragma once
#include <string>

// define my own protocol, convenient construction of said protocol,
// and write a parser for it too from a bytestream

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

} // namespace byoredis
