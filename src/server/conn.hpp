#pragma once

#include "./protocol/stl_wrapper.hpp"
#include "common.hpp"
#include <cstddef>
#include <sys/poll.h>
#include <variant>
#include <vector>

namespace byoredis {

constexpr uint32_t MAX_MSG_SIZE = 4096;

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
  Conn(UniqueFd client_fd, STLWrapper &db);

  pollfd construct_poll() const;
  void handle();
  bool is_closed() const;

private:
  UniqueFd client_fd;
  ConnState state;
  std::vector<std::byte> read_buffer;
  std::vector<std::byte> write_buffer;
  STLWrapper &db;

  friend struct State::Reading;
  friend struct State::Writing;
  friend struct State::Closed;
};

} // namespace byoredis
