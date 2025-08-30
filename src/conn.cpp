#include "conn.hpp"
#include "common.hpp"
#include <iostream>
#include <unistd.h>
#include <variant>

namespace byoredis {

namespace State {

Reading::Reading(Conn *conn) : conn(conn) {}
pollfd Reading::construct_poll() const {
  return {.fd = conn->client_fd.get(), .events = POLLIN, .revents = 0};
}
ConnState Reading::handle() { return *this; }
bool Reading::is_closed() const { return false; }

Writing::Writing(Conn *conn) : conn(conn) {}
pollfd Writing::construct_poll() const {
  return {.fd = conn->client_fd.get(), .events = POLLOUT, .revents = 0};
}
ConnState Writing::handle() { return *this; }
bool Writing::is_closed() const { return false; }

pollfd Closed::construct_poll() const { return {}; }
ConnState Closed::handle() { return *this; }
bool Closed::is_closed() const { return true; }

} // namespace State

Conn::Conn(UniqueFd client_fd)
    : client_fd(std::move(client_fd)), state(State::Reading(this)) {}

pollfd Conn::construct_poll() const {
  return std::visit([](auto &curr) { return curr.construct_poll(); }, state);
}

void Conn::handle() {
  state = std::visit([](auto &curr) { return curr.handle(); }, state);
}

bool Conn::is_closed() const {
  return std::visit([](auto &curr) { return curr.is_closed(); }, state);
}

} // namespace byoredis
