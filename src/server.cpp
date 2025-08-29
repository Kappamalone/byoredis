#include "server.hpp"
#include "conn.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/poll.h>

namespace byoredis {

namespace {

// copied verbatim from build-your-own-redis
/*
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
*/

ResultVoid set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    return err("Couldn't get flags for fd");
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    return err("Couldn't set fd to non blocking");
  }

  return ok();
}

} // namespace

Server::Server(int16_t port) : port(port) {}

ResultVoid Server::bind() {
  // AF_INET vs AF_INET6 : iPv4 vs iPv6
  // SOCKET_STREAM vs SOCK_DGRAM : TCP vs UDP
  UniqueFd fd{::socket(AF_INET, SOCK_STREAM, 0)};
  if (fd == -1) {
    return err("Failed to obtain socket fd");
  }

  // let socket rebind to same port after restart
  int val = 1;
  ::setsockopt(fd.get(), SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  struct sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(0);
  int res = ::bind(fd.get(), (const struct sockaddr *)&addr, sizeof(addr));
  if (res == -1) {
    return err("Failed to bind");
  }

  listen_fd = std::move(fd);
  return ok();
}

ResultVoid Server::listen() {
  if (listen_fd == -1) {
    return err("Socket not initialised");
  }

  if (auto e = set_nonblocking(listen_fd.get())) {
    return e;
  }

  // TODO: is it okay to call listen multiple times?
  int res = ::listen(listen_fd.get(), SOMAXCONN);
  if (res == -1) {
    return err("Failed to listen on socket");
  }

  // TODO: graceful shutdown?
  while (true) {

    std::vector<pollfd> poll_args = construct_polls();
    poll(poll_args.data(), (nfds_t)poll_args.size(), -1);

    // handle listen socket specifically
    if (poll_args[0].revents & POLLIN) {
      std::cout << "got a connection request!\n";

      struct sockaddr_in client_addr {};
      socklen_t addr_len = sizeof(client_addr);
      UniqueFd client_fd{::accept(listen_fd.get(),
                                  (struct sockaddr *)&client_addr, &addr_len)};

      if (client_fd == -1) {
        std::cerr << "Failed to accept client connection\n";
        continue;
      }

      if (auto e = set_nonblocking(client_fd.get())) {
        std::cerr << "client fd: " << *e << "\n";
        continue;
      }

      // create connection, create mapping between fd and connection
      int fd = client_fd.get();
      fd_to_connection.emplace(fd, std::move(client_fd));
      // do_something(client_fd.get());
      // close(client_fd.get());
    }

    for (size_t i = 1; i < poll_args.size(); ++i) {
      uint32_t revents = (uint32_t)poll_args[i].revents;
      Conn &conn = fd_to_connection.at(poll_args[i].fd);

      if ((revents & POLLIN) || (revents & POLLOUT)) {
        conn.handle();
      }

      if ((revents & POLLERR) || conn.is_closed()) {
        conn.close();
        fd_to_connection.erase(poll_args[i].fd);
      }
    }
  }

  return ok();
}

std::vector<pollfd> Server::construct_polls() {
  std::vector<pollfd> ret;
  ret.push_back({listen_fd.get(), POLL_IN, 0});

  for (const auto &[_, conn] : fd_to_connection) {
    ret.push_back(conn.construct_poll());
  }
  return ret;
}

} // namespace byoredis
