#include "conn.hpp"
#include "common.hpp"
#include "redis_handler.hpp"
#include "server/protocol.hpp"
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <variant>

namespace byoredis {

namespace State {

Reading::Reading(Conn* conn) : conn(conn) {}
pollfd Reading::construct_poll() const {
  return {.fd = conn->client_fd.get(), .events = POLLIN, .revents = 0};
}
ConnState Reading::handle() {
  std::byte tmp[64 * 1024];
  ssize_t bytes_read = read(conn->client_fd.get(), &tmp, sizeof(tmp));

  // -1 = error, 0 = EOF, either way we terminate the connection
  if (bytes_read <= 0) {
    return Closed();
  }

  conn->read_buffer.insert(conn->read_buffer.end(), tmp, tmp + bytes_read);

  // decode messages in a loop to handle pipelined requests
  while (!conn->read_buffer.empty()) {
    auto req = parse_request(conn->read_buffer);
    enum class Control { Request, Incomplete, Error };
    Control ctrl = std::visit(
        overloads{[&](const std::pair<size_t, Request>& req) {
                    auto [req_size, req_cmd] = req;
                    Response response = std::visit(
                        overloads{[&](const GetCommand& cmd) {
                                    return conn->db.get(cmd.key);
                                  },
                                  [&](const SetCommand& cmd) {
                                    return conn->db.set(cmd.key, cmd.value);
                                  },
                                  [&](const DeleteCommand& cmd) {
                                    return conn->db.del(cmd.key);
                                  }

                        },
                        req_cmd);

                    conn->read_buffer.erase(conn->read_buffer.begin(),
                                            conn->read_buffer.begin() +
                                                req_size);

                    std::vector<std::byte> out = serialise(response);
                    conn->write_buffer.insert(conn->write_buffer.end(),
                                              out.begin(), out.end());
                    return Control::Request;
                  },
                  [&](const Incomplete&) { return Control::Incomplete; },
                  [&](const Error& err) { return Control::Error; }},
        req);
    switch (ctrl) {
      case Control::Request:
        break; // keep parsing
      case Control::Incomplete:
        return *this; // wait for more bytes
      case Control::Error:
        return Closed(); // terminate connection
    }
  }

  return Writing(conn);
}
bool Reading::is_closed() const { return false; }

// ==============================================

Writing::Writing(Conn* conn) : conn(conn) {}
pollfd Writing::construct_poll() const {
  return {.fd = conn->client_fd.get(), .events = POLLOUT, .revents = 0};
}
ConnState Writing::handle() {
  ssize_t bytes_written =
      write(conn->client_fd.get(), conn->write_buffer.data(),
            conn->write_buffer.size());

  // our write side buffer is full, try again
  if (bytes_written == -1 && errno == EAGAIN) {
    return *this;
  }

  if (bytes_written == -1) {
    return Closed();
  }

  conn->write_buffer.erase(conn->write_buffer.begin(),
                           conn->write_buffer.begin() + bytes_written);
  if (!conn->write_buffer.empty()) {
    return *this;
  }

  return Reading(conn);
}
bool Writing::is_closed() const { return false; }

// ==============================================

pollfd Closed::construct_poll() const { return {}; }
ConnState Closed::handle() { return *this; }
bool Closed::is_closed() const { return true; }

} // namespace State

Conn::Conn(UniqueFd client_fd, STLWrapper& db)
    : client_fd(std::move(client_fd)), state(State::Reading(this)), db(db) {}

pollfd Conn::construct_poll() const {
  return std::visit([](auto& curr) { return curr.construct_poll(); }, state);
}

void Conn::handle() {
  state = std::visit([](auto& curr) { return curr.handle(); }, state);
}

bool Conn::is_closed() const {
  return std::visit([](auto& curr) { return curr.is_closed(); }, state);
}

} // namespace byoredis
