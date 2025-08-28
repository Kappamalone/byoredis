#pragma once

#include "common.hpp"

namespace byoredis {

// A redis server listens on a user-supplied port and handles requests in an
// event-driven loop
//
// Resources managed:
// - socket fd
class Server {
public:
  Server(int16_t port);

  ResultVoid bind();
  // event loop
  ResultVoid listen();

private:
  int16_t port;
  UniqueFd socket_fd;
};

} // namespace byoredis
