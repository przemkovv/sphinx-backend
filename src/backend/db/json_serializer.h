
#pragma once

#include "model.h"

#include <algorithm>
#include <nlohmann/json.hpp>

#include <sphinx_assert.h>

namespace Sphinx::Backend::Model {

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
  using Cols = Meta::Columns<Course>;
  return {{Cols::id_n, course.id},
          {Cols::name_n, course.name},
          {Cols::description_n, course.description.value_or(nullptr)}};
}

//----------------------------------------------------------------------
template <>
inline nlohmann::json to_json(const User &user)
{
  using Cols = Meta::Columns<User>;
  return {{Cols::id_n, user.id},
          {Cols::username_n, user.username},
          {Cols::email_n, user.email}};
}

//----------------------------------------------------------------------
template <>
inline nlohmann::json to_json(const Module &module)
{
  using Cols = Meta::Columns<Module>;
  return {{Cols::id_n, module.id},
          {Cols::course_id_n, module.course_id},
          {Cols::name_n, module.name},
          {Cols::description_n, module.description.value_or(nullptr)}};
}

//----------------------------------------------------------------------
template <typename E>
E from_json(const nlohmann::json & /* json */)
{
  static_assert(assert_false<E>::value, "Not implemented for the type.");
}

//----------------------------------------------------------------------
template <>
inline Course from_json(const nlohmann::json &data)
{
  using Cols = Meta::Columns<Course>;
  Course entity;
  entity.name = data[Cols::name_n];
  if (data[Cols::description_n].is_null()) {
    entity.description = nullopt;
  }
  else {
    entity.description = data[Cols::description_n].get<Cols::description_t>();
  }
  return entity;
}
template <>
inline Module from_json(const nlohmann::json &data)
{
  using Cols = Meta::Columns<Module>;
  Module entity;
  entity.name = data[Cols::name_n];
  if (data[Cols::description_n].is_null()) {
    entity.description = nullopt;
  }
  else {
    entity.description = data[Cols::description_n].get<Cols::description_t>();
  }
  entity.course_id = data[Cols::course_id_n];
  return entity;
}
template <>
inline User from_json(const nlohmann::json &data)
{
  using Cols = Meta::Columns<User>;
  User entity;
  entity.username = data[Cols::username_n];
  entity.email = data[Cols::email_n];
  return entity;
}

} // namespace Sphinx::Db
