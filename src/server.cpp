#include "server.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {

// copied verbatim from build-your-own-redis
void do_something(int connfd) {
  char rbuf[64] = {};
  ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
  if (n < 0) {
    return;
  }
  std::cout << "client says:" << rbuf << "\n";

  char wbuf[] = "world";
  write(connfd, wbuf, strlen(wbuf));
}

} // namespace

namespace byoredis {

Server::Server(int16_t port) : port(port) {}

Server::~Server() {
  if (socket_fd) {
    ::close(*socket_fd);
  }
}

ResultVoid Server::bind() {
  // AF_INET vs AF_INET6 : iPv4 vs iPv6
  // SOCKET_STREAM vs SOCK_DGRAM : TCP vs UDP
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    return err("Failed to obtain socket fd");
  }
  socket_fd = fd;

  // let socket rebind to same port after restart
  int val = 1;
  ::setsockopt(*socket_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  struct sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(0);
  int res = ::bind(*socket_fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (res == -1) {
    return err("Failed to bind");
  }

  return ok();
}

ResultVoid Server::listen() {
  if (!socket_fd) {
    return err("Socket not initialised");
  }

  // TODO: is it okay to call listen multiple times?
  int res = ::listen(*socket_fd, SOMAXCONN);
  if (res == -1) {
    return err("Failed to listen on socket");
  }

  // TODO: graceful shutdown?
  while (true) {
    struct sockaddr_in client_addr {};
    socklen_t addr_len = sizeof(client_addr);
    int client_fd =
        accept(*socket_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
      return err("Failed to accept client connection");
    }

    do_something(client_fd);
    close(client_fd);
  }

  return ok();
}

} // namespace byoredis
