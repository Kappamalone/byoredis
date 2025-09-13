#include "conn.hpp"
#include "common.hpp"
#include "redis_handler.hpp"
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <variant>

namespace byoredis {

namespace {

inline void put_be32(std::vector<std::byte>& buf, uint32_t v) {
  buf.push_back(static_cast<std::byte>(v >> 24));
  buf.push_back(static_cast<std::byte>(v >> 16));
  buf.push_back(static_cast<std::byte>(v >> 8));
  buf.push_back(static_cast<std::byte>(v));
}
} // namespace

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
  while (conn->read_buffer.size() >= sizeof(Command) + sizeof(size_t)) {
    Command cmd_type;
    std::memcpy(&cmd_type, conn->read_buffer.data(), sizeof(cmd_type));

    Response res;

    switch (cmd_type) {
      case Command::GET: {
        size_t nbytes;
        std::memcpy(&nbytes, conn->read_buffer.data() + sizeof(Command),
                    sizeof(nbytes));
        if (conn->read_buffer.size() - sizeof(Command) - sizeof(size_t) <
            nbytes) {
          return *this;
        }

        std::string key(nbytes, ' ');
        std::memcpy(key.data(),
                    conn->read_buffer.data() + sizeof(Command) + sizeof(nbytes),
                    nbytes);
        std::cout << "got a get cmd: " << key << "\n";
        res = conn->db.get(key);
        std::cout << "output: " << res.msg << "\n";
        conn->read_buffer.erase(conn->read_buffer.begin(),
                                conn->read_buffer.begin() + sizeof(size_t) +
                                    sizeof(cmd_type) + nbytes);
      }
      case Command::SET:
      case Command::DELETE:
        break;
    }

    put_be32(conn->write_buffer, res.msg.size());
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
