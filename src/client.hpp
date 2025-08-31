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

    char rbuf[64] = {};
    ssize_t n = read(server_fd.get(), rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
      return;
    }
    std::cout << "server says: " << rbuf + sizeof(size_t) << "\n";
  }

private:
  UniqueFd server_fd;
};

} // namespace byoredis
