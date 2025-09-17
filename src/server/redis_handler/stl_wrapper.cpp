#include "stl_wrapper.hpp"

namespace byoredis {

STLWrapper::STLWrapper() {}

Response STLWrapper::get_impl(std::string key) {
  auto it = db.find(key);
  if (it == db.end()) {
    return Response{.status = Status::ERROR, .msg = "could not find key"};
  }
  return Response{.status = Status::SUCCESS, .msg = it->second};
}

Response STLWrapper::set_impl(std::string key, std::string value) {
  auto [_, existed] = db.insert_or_assign(key, value);
  Response r = Response{.status = Status::SUCCESS, .msg = ""};
  if (existed) {
    r.msg = "Reassigned key";
  } else {
    r.msg = "Inserted key";
  }
  return r;
}

Response STLWrapper::del_impl(std::string key) {
  db.erase(key);
  return Response{.status = Status::SUCCESS, .msg = "erased key"};
}

}; // namespace byoredis
