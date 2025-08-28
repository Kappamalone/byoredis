#pragma once

#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace byoredis {

class Client {

public:
  // copied verbatim from build-your-own-redis
  static void dummy() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
      return;
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(3000);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1
    int err = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (err) {
      printf("oh no! 2");
      return;
    }

    char msg[] = "hello";
    write(fd, msg, strlen(msg));

    char rbuf[64] = {};
    ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
      return;
    }
    printf("server says: %s\n", rbuf);
    close(fd);
  }

private:
};

} // namespace byoredis
