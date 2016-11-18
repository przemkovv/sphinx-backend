
#pragma once

#include "data.h"

#include <algorithm>
#include <nlohmann/json.hpp>

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
inline nlohmann::json to_json(const Course &course)
{
  return {{Meta::ColumnsName<Course>::id, course.id},
          {"name", course.name},
          {"description", course.description.value_or(nullptr)}};
}
template <>
inline nlohmann::json to_json(const User &user)
{
  return {{"id", user.id}, {"username", user.username}, {"email", user.email}};
}

template <typename E>
E from_json(const nlohmann::json & /* json */)
{
  static_assert(assert_false<E>::value, "Not implemented for the type.");
}

template <>
inline User from_json(const nlohmann::json &data)
{
  User user;
  user.username = data["username"];
  user.email = data["email"];
  return user;
}

} // namespace Sphinx::Db
