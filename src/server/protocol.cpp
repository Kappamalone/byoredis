#include "protocol.hpp"
#include "server/common.hpp"
#include <iostream>
#include <optional>
#include <variant>
#include <vector>

namespace byoredis {

namespace {

template <typename T>
std::optional<T> read_le(const std::vector<std::byte>& buffer, size_t idx) {
  if (idx + sizeof(T) > buffer.size()) {
    return std::nullopt;
  }

  T value = 0;
  for (size_t i = 0; i < sizeof(T); ++i) {
    value |= static_cast<T>(buffer[idx + i]) << (i * 8);
  }
  return value;
}

template <typename T>
void write_le(std::vector<std::byte>& buffer, T value) {
  for (size_t i = 0; i < sizeof(T); ++i) {
    buffer.push_back(static_cast<std::byte>((value >> (i * 8)) & 0xff));
  }
}

} // namespace

ParseReqResult parse_request(const std::vector<std::byte>& buffer) {
  size_t idx = 0;

  auto magic = read_le<decltype(ProtocolHeader::magic)>(buffer, idx);
  if (!magic) {
    return Incomplete{};
  }
  if (magic != PROTOCOL_MAGIC) {
    return Error{.msg = "Incorrect protocol magic"};
  }
  idx += sizeof(ProtocolHeader::magic);

  auto cmd = read_le<std::underlying_type_t<decltype(ProtocolHeader::cmd)>>(
      buffer, idx);
  if (!cmd) {
    return Incomplete{};
  }
  idx += sizeof(ProtocolHeader::cmd);

  switch (static_cast<Command>(*cmd)) {
    case Command::GET: {
      auto nbytes = read_le<uint32_t>(buffer, idx);
      if (!nbytes) {
        return Incomplete{};
      }
      idx += sizeof(uint32_t);
      if (idx + *nbytes > buffer.size()) {
        return Incomplete{};
      }
      std::string key(reinterpret_cast<const char*>(buffer.data() + idx),
                      *nbytes);
      idx += *nbytes;

      return std::make_pair(idx, GetCommand{.key = std::move(key)});
    }
    case Command::SET:
      return std::make_pair(idx, SetCommand{.key = "", .value = ""});
    case Command::DELETE:
      return std::make_pair(idx, DeleteCommand{.key = ""});
    default:
      return Error{.msg = "Unrecognised command"};
  }
}

ParseResResult parse_response(const std::vector<std::byte>& buffer) {
  size_t idx = 0;

  auto status =
      read_le<std::underlying_type_t<decltype(Response::status)>>(buffer, idx);
  if (!status) {
    return Error{.msg = "Status missing"};
  }
  idx += sizeof(Response::status);

  auto nbytes = read_le<uint32_t>(buffer, idx);
  if (!nbytes) {
    return Error{.msg = "Length of msg missing"};
  }
  idx += sizeof(uint32_t);
  if (idx + *nbytes > buffer.size()) {
    std::cout << idx + *nbytes << " " << buffer.size() << "\n";
    return Error{.msg = "Msg length incorrect"};
  }
  std::string msg(reinterpret_cast<const char*>(buffer.data() + idx), *nbytes);
  idx += *nbytes;

  return Response{.status = static_cast<Status>(*status),
                  .msg = std::move(msg)};
}

std::vector<std::byte> serialise(Request req) {
  return std::visit(
      overloads{
          [](const GetCommand& cmd) {
            std::vector<std::byte> out;
            write_le<decltype(ProtocolHeader::magic)>(out, PROTOCOL_MAGIC);
            write_le<std::underlying_type_t<decltype(ProtocolHeader::cmd)>>(
                out, static_cast<uint8_t>(Command::GET));
            write_le<uint32_t>(out, static_cast<uint32_t>(cmd.key.size()));
            out.insert(out.end(),
                       reinterpret_cast<const std::byte*>(cmd.key.data()),
                       reinterpret_cast<const std::byte*>(cmd.key.data() +
                                                          cmd.key.size()));
            return out;
          },
          [](const SetCommand& cmd) { return std::vector<std::byte>{}; },
          [](const DeleteCommand& cmd) { return std::vector<std::byte>{}; }},
      req);
}

std::vector<std::byte> serialise(Response req) {
  std::vector<std::byte> out;

  write_le<std::underlying_type_t<decltype(Response::status)>>(
      out, static_cast<uint8_t>(req.status));
  write_le<uint32_t>(out, static_cast<uint32_t>(req.msg.size()));
  out.insert(
      out.end(), reinterpret_cast<const std::byte*>(req.msg.data()),
      reinterpret_cast<const std::byte*>(req.msg.data() + req.msg.size()));

  return out;
}

} // namespace byoredis
