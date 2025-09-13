#include "stl_wrapper.hpp"

namespace byoredis {

STLWrapper::STLWrapper() {}

Response STLWrapper::get(std::string key) { return Response{.msg = "TODO"}; }
Response STLWrapper::set(std::string key, std::string value) {
  return Response{.msg = "TODO"};
}
Response STLWrapper::del(std::string key) { return Response{.msg = "TODO"}; }

}; // namespace byoredis
