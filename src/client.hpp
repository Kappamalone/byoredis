#pragma once

#include "common.hpp"
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace byoredis {

class Client {
public:
  Client();
  ResultVoid connect(int32_t host, int16_t port);
  void close();

  // placeholder
  void dummy() {
    char msg[] = "hi";
    std::cout << "msg len: " << strlen(msg) << "\n";
    size_t size = strlen(msg);
    write(server_fd.get(), &size, sizeof(size));
    write(server_fd.get(), msg, size);

    char rbuf[128] = {};
    ssize_t n = read(server_fd.get(), rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
      return;
    }
    std::cout << "server says: " << rbuf + sizeof(size_t) << "\n";
  }

  // pipelined request
  void dummy2() {
    char msg1[] = "hello";
    char msg2[] = "world";
    size_t size1 = strlen(msg1);
    size_t size2 = strlen(msg2);
    write(server_fd.get(), &size1, sizeof(size1));
    write(server_fd.get(), msg1, size1);
    write(server_fd.get(), &size2, sizeof(size2));
    write(server_fd.get(), msg2, size2);

    char rbuf[128] = {};
    ssize_t n = read(server_fd.get(), rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
      return;
    }
    ssize_t msg_size;
    memcpy(&msg_size, rbuf, sizeof(msg_size));
    std::cout << "server says: " << rbuf + sizeof(size_t) << "\n";
    std::cout << "server says: "
              << rbuf + sizeof(size_t) + msg_size + sizeof(size_t) << "\n";
  }

private:
  UniqueFd server_fd;
};

} // namespace byoredis
