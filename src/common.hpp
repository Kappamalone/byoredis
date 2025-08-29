#pragma once

#include <fcntl.h>
#include <optional>
#include <poll.h>
#include <string>
#include <sys/fcntl.h>
#include <unistd.h>

namespace byoredis {

// poor man's result
using ResultVoid = std::optional<std::string>;

// these helpers aren't necessary, but they make the code nicer to read
inline ResultVoid ok() { return std::nullopt; }
template <typename T> inline ResultVoid err(T &&e) {
  return std::make_optional<std::string>(std::forward<T>(e));
}

// RAII wrapper around fd's + convenience methods
// Prevents us from having to do something like `std::optional<int>`
class UniqueFd {
public:
  UniqueFd() = default;
  explicit UniqueFd(int fd) : fd(fd){};
  ~UniqueFd() {
    if (fd != -1) {
      ::close(fd);
    }
  }
  UniqueFd(const UniqueFd &other) = delete;
  UniqueFd &operator=(const UniqueFd &other) = delete;
  UniqueFd(UniqueFd &&other) noexcept : fd(std::exchange(other.fd, -1)) {}
  UniqueFd &operator=(UniqueFd &&other) noexcept {
    if (this != &other) {
      if (fd != -1) {
        ::close(fd);
      }
      fd = std::exchange(other.fd, -1);
    }
    return *this;
  }

  int get() const { return fd; }
  auto operator==(int other) const { return fd == other; }

private:
  int fd = -1;
};

} // namespace byoredis
