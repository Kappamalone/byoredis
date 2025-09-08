#include "client.hpp"
#include "../server/common.hpp"
#include <cstring>
#include <iostream>

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
  int res = ::connect(fd.get(), (const struct sockaddr *)&addr, sizeof(addr));
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
  std::byte request[1024];
  Command request_type = Command::GET;
  std::memcpy(&request, &request_type, sizeof(request_type));

  size_t key_len = key.size();
  std::memcpy(request + sizeof(request_type), &key_len, sizeof(key_len));

  std::memcpy(request + sizeof(request_type) + sizeof(key_len), key.data(),
              key.size());

  write(server_fd.get(), &request,
        sizeof(request_type) + sizeof(key_len) + key.size());
  std::cout << "done!\n";

  std::byte response[1024] = {};
  std::cout << "reading from server\n";
  ssize_t n = read(server_fd.get(), response, sizeof(response) - 1);
  std::cout << "done!\n";
  if (n < 0) {
    return {.msg = "failed to read"};
  }

  std::cout << n << "\n";
  return {.msg = "woohoo"};
}

} // namespace byoredis
