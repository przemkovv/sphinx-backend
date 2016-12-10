
#pragma once

#include "logger.h"
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
  /* clang-format off */
  if constexpr(is_column(value))
  {
    if constexpr(is_optional(value))
    {
      if (value.value)
        return {value.name, *value.value};
      else
        return {value.name, nullptr};
    }
    else {
      return {value.name, value.value};
    }
  }
  else {
    return {value};
  }
  /* clang-format on */
}

template <typename... T>
nlohmann::json to_json(const T &... value)
{
  return {to_json(value)...};
}

template <typename... T>
nlohmann::json to_json(const std::tuple<T ...>& value)
{
  return std::apply(to_json<T...>, value);
}

//----------------------------------------------------------------------
template <template <typename, typename> typename C, typename E, typename A>
nlohmann::json to_json(const C<E, A> &c)
{
  global_logger->debug("to_json<Container>");
  nlohmann::json json;
  std::transform(c.begin(), c.end(), std::back_inserter(json),
                 [](const auto &entity) { return to_json(entity); });
  return json;
}

//----------------------------------------------------------------------
template <>
inline nlohmann::json to_json(const Backend::Model::Course &course)
{
  global_logger->debug("to_json<Course>: {}", course.name.value);
  return to_json(course.get_columns());
}

//----------------------------------------------------------------------
template <>
inline nlohmann::json to_json(const Backend::Model::User &user)
{
  global_logger->debug("to_json<User>: {}", user.firstname.value);
  return to_json(user.get_columns());
}

//----------------------------------------------------------------------
template <>
inline nlohmann::json to_json(const Backend::Model::Module &module)
{
  global_logger->debug("to_json<Module>: {}", module.name.value);
return to_json(module.id,
        module.course_id,
        module.name,
        module.description);
}

//----------------------------------------------------------------------
template <typename E>
E from_json(const nlohmann::json & /* json */)
{
  static_assert(assert_false<E>::value, "Not implemented for the type.");
}

//----------------------------------------------------------------------
template <int N, typename E, typename T, auto Name, typename... Ts>
auto from_json(const nlohmann::json &data,
               const Db::Column<N, E, T, Name, Ts...> &column)
{
  return data[column.name].template get<T>();
}

//----------------------------------------------------------------------
template <int N, typename E, typename T, auto Name, typename... Traits>
void load_from_json(Db::Column<N, E, T, Name, Traits...> &column,
                    const nlohmann::json &data)
{
  if
    constexpr(Db::is_optional(column))
    {
      if (data.count(column.name) == 0 || data[column.name].is_null()) {
        column.value = std::nullopt;
        return;
      }
    }
  column.value = data[Name].template get<T>();
}

//----------------------------------------------------------------------
template <>
inline Backend::Model::Course from_json(const nlohmann::json &data)
{
  global_logger->debug("from_json<Course>");
  Backend::Model::Course entity;
  load_from_json(entity.name, data);
  load_from_json(entity.description, data);
  return entity;
}
//----------------------------------------------------------------------
template <>
inline Backend::Model::Module from_json(const nlohmann::json &data)
{
  global_logger->debug("from_json<Module>");
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
  global_logger->debug("from_json<User>");
  Backend::Model::User entity;
  load_from_json(entity.username, data);
  load_from_json(entity.firstname, data);
  load_from_json(entity.lastname, data);
  load_from_json(entity.student_id, data);
  load_from_json(entity.role, data);
  load_from_json(entity.email, data);
  return entity;
}

} // namespace Sphinx::Db
