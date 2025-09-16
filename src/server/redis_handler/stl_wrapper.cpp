#include "stl_wrapper.hpp"

namespace byoredis {

STLWrapper::STLWrapper() {}

Response STLWrapper::get_impl(std::string key) {
  return Response{.msg = "finally got my very own protocol"};
}
Response STLWrapper::set_impl(std::string key, std::string value) {
  return Response{.msg = "TODO"};
}
Response STLWrapper::del_impl(std::string key) {
  return Response{.msg = "TODO"};
}

}; // namespace byoredis
