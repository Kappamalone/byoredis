#pragma once

#include "common.hpp"
#include <optional>

namespace byoredis {

// A redis server listens on a user-supplied port and handles requests in an
// event-driven loop
//
// Resources managed:
// - socket fd
class Server {
public:
  Server(int16_t port);

  // for now, let's define our destructor to clean up the socket
  // and delete all the special ctors
  ~Server();
  Server(const Server &other) = delete;
  Server operator=(const Server &other) = delete;
  Server(Server &&other) noexcept = delete;
  Server operator=(Server &&other) noexcept = delete;

  ResultVoid bind();
  // event loop
  ResultVoid listen();

private:
  int16_t port;
  std::optional<int> socket_fd;
};

} // namespace byoredis
