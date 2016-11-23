
#pragma once

#include "model.h"
#include "model_utils.h"

#include <algorithm>
#include <nlohmann/json.hpp>

#include <sphinx_assert.h>

namespace Sphinx::Db::Json {

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
inline nlohmann::json to_json(const Backend::Model::Course &course)
{
  using Cols = Meta::Columns<Backend::Model::Course>;
  return {
      {course.id.name, course.id.value},
      {course.name.name, course.name.value},
      {course.description.name, course.description.value.value_or(nullptr)}};
}

//----------------------------------------------------------------------
template <>
inline nlohmann::json to_json(const Backend::Model::User &user)
{
  using Cols = Meta::Columns<Backend::Model::User>;
  return {{user.id.name, user.id.value},
          {user.username.name, user.username.value},
          {user.email.name, user.email.value}};
}

//----------------------------------------------------------------------
template <>
inline nlohmann::json to_json(const Backend::Model::Module &module)
{
  using Cols = Meta::Columns<Backend::Model::Module>;
  return {
      {module.id.name, module.id.value},
      {module.course_id.name, module.course_id.value},
      {module.name.name, module.name.value},
      {module.description.name, module.description.value.value_or(nullptr)}};
}

//----------------------------------------------------------------------
template <typename E>
E from_json(const nlohmann::json & /* json */)
{
  static_assert(assert_false<E>::value, "Not implemented for the type.");
}

//----------------------------------------------------------------------

template <typename E, typename T, auto N>
auto from_json(const nlohmann::json &data,
               const Db::Meta::Column<E, T, N> &column)
{
  return data[column.name].template get<T>();
}

template <>
inline Backend::Model::Course from_json(const nlohmann::json &data)
{
  using Cols = Meta::Columns<Backend::Model::Course>;
  Backend::Model::Course entity;
  entity.name.value = from_json(data, entity.name);
  if (data[entity.description.name].is_null()) {
    entity.description.value = Db::nullopt;
  }
  else {
    entity.description.value = from_json(data, entity.description);
  }
  return entity;
}
template <>
inline Backend::Model::Module from_json(const nlohmann::json &data)
{
  using Cols = Meta::Columns<Backend::Model::Module>;
  Backend::Model::Module entity;
  entity.name.value = from_json(data, entity.name);
  if (data[entity.description.name].is_null()) {
    entity.description.value = Db::nullopt;
  }
  else {
    entity.description.value = from_json(data, entity.description);
  }
  entity.course_id.value = from_json(data, entity.course_id);
  return entity;
}
template <>
inline Backend::Model::User from_json(const nlohmann::json &data)
{
  using Cols = Meta::Columns<Backend::Model::User>;
  Backend::Model::User entity;
  entity.username.value = from_json(data, entity.username);
  entity.email.value = from_json(data, entity.email);
  return entity;
}

} // namespace Sphinx::Db
