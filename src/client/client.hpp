#pragma once

#include "../server/common.hpp"
#include "../server/protocol.hpp"
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace byoredis {

class Client {
public:
  Client();
  ResultVoid connect(int32_t host, int16_t port);

  Response get(std::string key);
  Response set(std::string key, std::string value);
  Response del(std::string key);

private:
  Response hit_server(Request req);

  UniqueFd server_fd;
};

} // namespace byoredis
