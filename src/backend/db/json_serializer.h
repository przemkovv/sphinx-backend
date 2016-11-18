
#pragma once

#include "data.h"

#include <algorithm>
#include <nlohmann/json.hpp>

#include <sphinx_assert.h>

namespace Sphinx::Db {

//----------------------------------------------------------------------
template <typename C>
nlohmann::json to_json(const C &c)
{
  nlohmann::json json;
  std::transform(c.begin(), c.end(), std::back_inserter(json),
                 [](const auto &entity) { return to_json(entity); });
  return json;
}

//----------------------------------------------------------------------
template <>
inline nlohmann::json to_json(const Course &course)
{
  using Name = Meta::ColumnsName<Course>;
  return {{Name::id, course.id},
          {Name::name, course.name},
          {Name::description, course.description.value_or(nullptr)}};
}

//----------------------------------------------------------------------
template <>
inline nlohmann::json to_json(const User &user)
{
  using Name = Meta::ColumnsName<User>;
  return {{Name::id, user.id},
          {Name::username, user.username},
          {Name::email, user.email}};
}

//----------------------------------------------------------------------
template <typename E>
E from_json(const nlohmann::json & /* json */)
{
  static_assert(assert_false<E>::value, "Not implemented for the type.");
}

//----------------------------------------------------------------------
template <>
inline User from_json(const nlohmann::json &data)
{
  using Name = Meta::ColumnsName<User>;
  User user;
  user.username = data[Name::username];
  user.email = data[Name::email];
  return user;
}

} // namespace Sphinx::Db
