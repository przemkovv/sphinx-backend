
#pragma once

#include "model.h"           // for Module, Course, User, Column
#include "model_meta.h"      // for Column<>::type, Column, nullopt
#include "sphinx_assert.h"   // for assert_false
#include <algorithm>         // for transform
#include <iterator>          // for back_inserter
#include <nlohmann/json.hpp> // for basic_json, json
#include <optional>          // for optional, nullopt

namespace Sphinx::Db::Json {

//----------------------------------------------------------------------
template <typename T>
nlohmann::json to_json(const T &value)
{
  return {value};
}

//----------------------------------------------------------------------
template <template <typename, typename> typename C, typename E, typename A>
nlohmann::json to_json(C<E, A> &&c)
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
  return {
      {course.id.name, course.id.value},
      {course.name.name, course.name.value},
      {course.description.name, course.description.value.value_or(nullptr)}};
}

//----------------------------------------------------------------------
template <>
inline nlohmann::json to_json(const Backend::Model::User &user)
{
  return {{user.id.name, user.id.value},
          {user.username.name, user.username.value},
          {user.email.name, user.email.value}};
}

//----------------------------------------------------------------------
template <>
inline nlohmann::json to_json(const Backend::Model::Module &module)
{
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
template <typename E, typename T, auto N, typename... Ts>
auto from_json(const nlohmann::json &data,
               const Db::Column<E, T, N, Ts...> &column)
{
  return data[column.name].template get<T>();
}

//----------------------------------------------------------------------
template <typename E, typename T, auto N, typename... Traits>
void load_from_json(Db::Column<E, T, N, Traits...> &column,
                    const nlohmann::json &data)
{
  if
    constexpr(Db::is_optional(column))
    {
      if (data[column.name].is_null()) {
        column.value = std::nullopt;
        return;
      }
    }
  column.value = data[N].template get<T>();
}

//----------------------------------------------------------------------
template <>
inline Backend::Model::Course from_json(const nlohmann::json &data)
{
  Backend::Model::Course entity;
  load_from_json(entity.name, data);
  load_from_json(entity.description, data);
  return entity;
}
//----------------------------------------------------------------------
template <>
inline Backend::Model::Module from_json(const nlohmann::json &data)
{
  Backend::Model::Module entity;
  load_from_json(entity.name, data);
  load_from_json(entity.description, data);
  load_from_json(entity.course_id, data);
  return entity;
}
//----------------------------------------------------------------------
template <>
inline Backend::Model::User from_json(const nlohmann::json &data)
{
  Backend::Model::User entity;
  load_from_json(entity.username, data);
  load_from_json(entity.email, data);
  return entity;
}

} // namespace Sphinx::Db
