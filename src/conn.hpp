#pragma once

#include "common.hpp"
#include <sys/poll.h>
#include <variant>

namespace byoredis {

class Conn;

namespace State {
struct Reading;
struct Writing;
struct Closed;
} // namespace State

using ConnState = std::variant<State::Reading, State::Writing, State::Closed>;

namespace State {

struct Reading {
  Reading(Conn *conn);
  pollfd construct_poll() const;
  ConnState handle();
  bool is_closed() const;

private:
  Conn *conn;
};

struct Writing {
  Writing(Conn *conn);
  pollfd construct_poll() const;
  ConnState handle();
  bool is_closed() const;

private:
  Conn *conn;
};

struct Closed {
  pollfd construct_poll() const;
  ConnState handle();
  bool is_closed() const;
};

} // namespace State

// Represents a connection with a client
class Conn {
public:
  Conn(UniqueFd client_fd);

  pollfd construct_poll() const;
  void handle();
  bool is_closed() const;

private:
  UniqueFd client_fd;
  ConnState state;

  friend struct State::Reading;
  friend struct State::Writing;
  friend struct State::Closed;
};

} // namespace byoredis
