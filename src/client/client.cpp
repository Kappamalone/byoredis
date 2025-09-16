#include "client.hpp"
#include "../server/common.hpp"
#include "../server/protocol.hpp"
#include <cstring>
#include <iostream>
#include <vector>

namespace byoredis {

Client::Client() {}

ResultVoid Client::connect(int32_t host, int16_t port) {
  UniqueFd fd{::socket(AF_INET, SOCK_STREAM, 0)};
  if (fd == -1) {
    return err("Failed to obtain socket fd");
  }

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(port);
  addr.sin_addr.s_addr = ntohl(host);
  int res = ::connect(fd.get(), (const struct sockaddr*)&addr, sizeof(addr));
  if (res) {
    return err("Failed to bind");
  }

  server_fd = std::move(fd);

  return ok();
}

// a request is defined by:
// uint32_t command type (defined in protocol.hpp)
// uint32_t num bytes for string
// ... string bytes
Response Client::get(std::string key) {
  GetCommand cmd{.key = key};
  std::vector<std::byte> payload = serialise(cmd);
  write(server_fd.get(), payload.data(), payload.size());

  std::vector<std::byte> buffer;
  buffer.resize(1024);
  std::cout << "reading from server\n";
  ssize_t n = read(server_fd.get(), buffer.data(), buffer.size());
  if (n < 0) {
    return {.status = Status::ERROR, .msg = "failed to read"};
  }

  auto result = parse_response(buffer);
  if (std::holds_alternative<Error>(result)) {
    return {.status = Status::ERROR,
            .msg = std::move(std::get<Error>(result).msg)};
  }

  return std::get<Response>(result);
}

} // namespace byoredis
