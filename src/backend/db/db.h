
#pragma once

#include "tables.h"

#include <json_diag.hpp>

#include <sqlpp11/sqlite3/sqlite3.h>

#include <algorithm>
#include <iterator>
#include <vector>

namespace Sphinx::Db {

using connection_config = sqlpp::sqlite3::connection_config;
using connection = sqlpp::sqlite3::connection;

struct User {
  int64_t id;
  std::string username;
  std::string email;
};

template <typename C>
nlohmann::json to_json(const C &container)
{
  nlohmann::json json;
  std::transform(container.begin(), container.end(), std::back_inserter(json),
                 [](const auto &entity) { return to_json(entity); });
  return json;
}

template <>
inline nlohmann::json to_json(const User &user)
{
  return {{"id", user.id}, {"username", user.username}, {"email", user.email}};
}

template <typename UserRow>
User to_user(const UserRow &user_row)
{
  return {user_row.id, user_row.username, user_row.email};
}

template <typename DB>
std::vector<User> get_users(DB &db);

extern template std::vector<User> get_users(sqlpp::sqlite3::connection &db);

} // namespace Sphinx::Db
