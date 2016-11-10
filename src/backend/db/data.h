#pragma once

#include <string>

namespace Sphinx::Db {

struct User {
  int64_t id;
  std::string username;
  std::string email;
};

} // namespace Sphinx::Db
