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
    char msg[] = "hello";
    write(server_fd.get(), msg, strlen(msg));

    char rbuf[64] = {};
    ssize_t n = read(server_fd.get(), rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
      return;
    }
    std::cout << "server says: " << rbuf << "\n";
  }

private:
  UniqueFd server_fd;
};

} // namespace byoredis
