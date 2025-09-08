#include "./client/client.hpp"
#include "./server/server.hpp"
#include <cstring>
#include <iostream>

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "specify 1 for server, 2 for client\n";
    return 1;
  }

  if (strcmp(argv[1], "1") == 0) {
    byoredis::Server server{3000};

    if (auto e = server.bind()) {
      std::cerr << *e << "\n";
      return 1;
    }

    if (auto e = server.listen()) {
      std::cerr << *e << "\n";
      return 1;
    }
  } else {
    byoredis::Client client{};
    client.connect(INADDR_LOOPBACK, 3000);
    std::cout << client.get("test").msg << "\n";
  }
  return 0;
}
