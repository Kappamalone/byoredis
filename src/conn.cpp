#include "conn.hpp"
#include "common.hpp"

namespace byoredis {

Conn::Conn(UniqueFd client_fd) : client_fd(std::move(client_fd)) {}

pollfd Conn::construct_poll() const { return {}; }
void Conn::handle() {}
bool Conn::is_closed() const { return true; }
void Conn::close() {}

} // namespace byoredis
