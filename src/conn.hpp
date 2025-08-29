#pragma once

#include "common.hpp"
#include <sys/poll.h>

namespace byoredis {

// Represents a connection with a client
class Conn {
public:
  Conn(UniqueFd client_fd);

  pollfd construct_poll() const;
  void handle();
  bool is_closed() const;
  void close();

private:
  enum State { Reading, Writing, Closed };

  UniqueFd client_fd;
  State state = State::Reading;
};

} // namespace byoredis
