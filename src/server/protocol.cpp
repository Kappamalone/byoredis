#include "protocol.hpp"
#include "server/common.hpp"
#include <iostream>
#include <optional>
#include <string_view>
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

std::optional<std::pair<size_t, std::string>>
parse_string(const std::vector<std::byte>& buffer, size_t idx) {
  size_t initial = idx;
  auto nbytes = read_le<uint32_t>(buffer, idx);
  if (!nbytes) {
    return std::nullopt;
  }
  idx += sizeof(uint32_t);
  if (idx + *nbytes > buffer.size()) {
    return std::nullopt;
  }
  std::string key(reinterpret_cast<const char*>(buffer.data() + idx), *nbytes);
  idx += *nbytes;
  return std::make_pair(idx - initial, key);
}

void serialise_string(std::vector<std::byte>& buffer, std::string_view str) {
  write_le<uint32_t>(buffer, static_cast<uint32_t>(str.size()));
  buffer.insert(buffer.end(), reinterpret_cast<const std::byte*>(str.data()),
                reinterpret_cast<const std::byte*>(str.data() + str.size()));
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
      auto key_value = parse_string(buffer, idx);
      if (!key_value) {
        return Incomplete{};
      }
      auto [nbytes, key] = *key_value;
      idx += nbytes;
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

  auto value = parse_string(buffer, idx);
  if (!value) {
    return Error{.msg = "Failed to parse string"};
  }
  auto [size, msg] = *value;

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
            serialise_string(out, cmd.key);
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
  serialise_string(out, req.msg);

  return out;
}

} // namespace byoredis
