#pragma once
#include <cstddef>
#include <string>
#include <variant>

// define my own protocol, convenient construction of said protocol,
// and write a parser for it too from a bytestream

namespace byoredis {

enum class Status : uint8_t {
  SUCCESS = 1,
  ERROR = 2,
};

struct Response {
  Status status;
  std::string msg;
};

enum class Command : uint8_t {
  GET = 1,
  SET = 2,
  DELETE = 3,
};

// actual protocols would include things like a magic number,
// version, reserved bytes etc, but we're going to forgo that for the
// sake of simplicity
constexpr uint16_t PROTOCOL_MAGIC = 0xFA12;
struct ProtocolHeader {
  uint16_t magic;
  Command cmd;
};

struct Incomplete {};
struct Error {
  std::string msg;
};
struct GetCommand {
  std::string key;
};
struct SetCommand {
  std::string key;
  std::string value;
};
struct DeleteCommand {
  std::string key;
};

using Request = std::variant<GetCommand, SetCommand, DeleteCommand>;
using ParseReqResult =
    std::variant<std::pair<size_t, Request>, Incomplete, Error>;
using ParseResResult = std::variant<Response, Error>;

std::vector<std::byte> serialise(Request req);
std::vector<std::byte> serialise(Response req);
ParseReqResult parse_request(const std::vector<std::byte>& buffer);
ParseResResult parse_response(const std::vector<std::byte>& buffer);

} // namespace byoredis
