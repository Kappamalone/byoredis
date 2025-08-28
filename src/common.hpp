#pragma once

#include <optional>
#include <string>

namespace byoredis {

// poor man's result
using ResultVoid = std::optional<std::string>;

// these helpers aren't necessary, but they make the code nicer to read
inline ResultVoid ok() { return std::nullopt; }
template <typename T> inline ResultVoid err(T &&e) {
  return std::make_optional<std::string>(std::forward<T>(e));
}

} // namespace byoredis
