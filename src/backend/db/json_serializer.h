
#pragma once

#include "data.h"

#include <algorithm>
#include <nlohmann/json.hpp>

#include <sqlpp11/sqlpp11.h>

#include <sphinx_assert.h>

namespace Sphinx::Db {

template <typename C>
nlohmann::json to_json(const C &c)
{
  nlohmann::json json;
  std::transform(c.begin(), c.end(), std::back_inserter(json),
                 [](const auto &entity) { return to_json(entity); });
  return json;
}

template <>
inline nlohmann::json to_json(const User &user)
{
  return {{"id", user.id}, {"username", user.username}, {"email", user.email}};
}

template <typename E>
E from_json(const nlohmann::json &/* json */)
{
  static_assert( assert_false<E>::value, "Not implemented for the type.");
}

template <>
inline User from_json(const nlohmann::json &data )
{
  User user;
  user.username = data["username"];
  user.email = data["email"];
  return user;
}

template <typename Row>
nlohmann::json row_to_json(const Row &row)
{
  nlohmann::json json_row;
  sqlpp::for_each_field(row, [&](const auto &field) {
    json_row[sqlpp::get_sql_name(field)] = field.value();
  });
  return json_row;
}

template <typename Rows>
nlohmann::json rows_to_json(Rows &rows)
{
  nlohmann::json json_rows;
  std::transform(rows.begin(), rows.end(), std::back_inserter(json_rows),
                 [](const auto &row) { return row_to_json(row); });
  return json_rows;
}

} // namespace Sphinx::Db
