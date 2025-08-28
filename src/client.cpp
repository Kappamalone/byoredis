#include "client.hpp"
#include "common.hpp"

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

void Client::close() { server_fd = UniqueFd{-1}; }

} // namespace byoredis
