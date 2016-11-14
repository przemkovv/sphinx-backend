
#pragma once

#include "data.h"
#include "tables.h"

#include <nlohmann/json.hpp>

#include <sqlpp11/sqlpp11.h>
#include <sqlpp11/sqlite3/sqlite3.h>

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

namespace Sphinx::Db {

using connection_config = sqlpp::sqlite3::connection_config;
using connection = sqlpp::sqlite3::connection;

class Db {

public:
  Db(const connection_config &db_config);

private:
  const connection_config db_config_;
  connection db_;

public:
  std::vector<User> get_users();
  nlohmann::json get_users_json();
  void create_user(const User &user);

private:
  auto get_users_query()
  {
    const auto users = Tables::Users{};
    const auto total = static_cast<std::size_t>(
        db_(select(sqlpp::count(users.id)).from(users).unconditionally())
            .front()
            .count);
    auto rows = db_(sqlpp::select(all_of(users)).from(users).unconditionally());
    return std::make_tuple(total, std::move(rows));
  }
};

} // namespace Sphinx::Db
