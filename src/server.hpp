#pragma once

#include "common.hpp"
#include "conn.hpp"
#include <sys/poll.h>
#include <unordered_map>
#include <vector>

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
  std::vector<pollfd> construct_polls();

  int16_t port;
  UniqueFd listen_fd;
  std::unordered_map<int, Conn> fd_to_connection;
};

} // namespace byoredis
