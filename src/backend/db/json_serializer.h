
#pragma once

#include "logger.h"          // for Logger, global_logger
#include "model_meta.h"      // for Column, is_optional, is_column
#include "utils.h"           // for for_each_in_tuple
#include <algorithm>         // for transform
#include <iterator>          // for back_inserter
#include <memory>            // for __shared_ptr_access
#include <nlohmann/json.hpp> // for json
#include <optional>          // for nullopt
#include <spdlog/spdlog.h>   // for logger
#include <tuple>             // for tuple, apply
#include <vector>            // for vector

namespace Sphinx::Db::Json {

// TODO: sth weird is with ordering of the these template functions. If I
// uncomment the vector overload, the code is not compiling.
template <typename T>
nlohmann::json to_json(const T &value);
template <typename... T>
nlohmann::json to_json(const T &... value);
template <typename... T>
nlohmann::json to_json(const std::tuple<T...> &value);
// template <typename T>
// nlohmann::json to_json(const std::vector<T> &c);

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
  else if constexpr (Meta::is_entity_v<T>) {
    return to_json(value.get_columns());
  }
  else {
    return to_json(value);

  }
  /* clang-format on */
}

//----------------------------------------------------------------------
template <typename... T>
nlohmann::json to_json(const T &... value)
{
  return {to_json(value)...};
}

//----------------------------------------------------------------------
template <typename... T>
nlohmann::json to_json(const std::tuple<T...> &value)
{
  return std::apply(to_json<T...>, value);
}

//----------------------------------------------------------------------
template <typename T>
nlohmann::json to_json(const std::vector<T> &c)
{
  global_logger->debug("to_json<Container>");
  nlohmann::json json;
  std::transform(c.begin(), c.end(), std::back_inserter(json),
                 [](const auto &entity) { return to_json(entity); });
  return json;
}

//----------------------------------------------------------------------
template <int N, typename E, typename T, auto Name, typename... Traits>
void load_from_json(Db::Column<N, E, T, Name, Traits...> &column,
                    const nlohmann::json &data)
{
  /* clang-format off */
  if constexpr(Db::is_primarykey(column)) {
    return;
  } 
  else if constexpr(Db::is_optional(column))
  {
    if (data.count(column.name) == 0 || data[column.name].is_null()) {
      column.value = std::nullopt;
      return;
    }
  } else if constexpr(Db::is_foreignkey(column)) {
    if (data.count(column.name) == 0) {
      return;
    }
  }
  column.value = data[Name].template get<T>();
  /* clang-format on */
}

//----------------------------------------------------------------------
template <typename E>
E from_json(const nlohmann::json &data)
{
  E entity;
  auto func = [&data](auto &col) { load_from_json(col, data); };
  auto cols = entity.get_columns();
  Sphinx::Utils::for_each_in_tuple(cols, func);
  return entity;
}

//----------------------------------------------------------------------
template <int N, typename E, typename T, auto Name, typename... Ts>
auto from_json(const nlohmann::json &data,
               const Db::Column<N, E, T, Name, Ts...> &column)
{
  return data[column.name].template get<T>();
}

} // namespace Sphinx::Db
