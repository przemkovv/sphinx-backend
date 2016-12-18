
#pragma once

#include "db/model_meta.h"     // for Column, is_optional, is_column
#include "shared_lib/logger.h" // for Logger, global_logger
#include <algorithm>           // for transform
#include <iterator>            // for back_inserter
#include <memory>              // for __shared_ptr_access
#include <nlohmann/json.hpp>   // for json
#include <spdlog/spdlog.h>     // for logger
#include <tuple>               // for tuple, apply
#include <vector>              // for vector

#include <boost/hana/accessors.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/fuse.hpp>
#include <boost/hana/members.hpp>

namespace Sphinx::Json {

namespace Meta = Sphinx::Db::Meta;

template <typename T>
nlohmann::json to_json(const T &value);
template <typename... T>
nlohmann::json to_json(const T &... value);
template <typename... T>
nlohmann::json to_json(const std::tuple<T...> &value);
template <typename T>
nlohmann::json to_json(const std::vector<T> &c);

//----------------------------------------------------------------------
template <typename T>
nlohmann::json to_json(const T &value)
{
  /* clang-format off */
  if constexpr(std::is_integral_v<T> || std::is_fundamental_v<T>) {
    return {{"value", value}};
  } else
  if constexpr(Meta::is_entity_v<T> || Meta::is_the_entity_v<T>)
  {
    nlohmann::json data;
    namespace hana = boost::hana;
    const auto &entity = value;

    auto column_to_json = [&data, &entity](const auto &name_c,
                                          const auto &member_ptr) {
      using type = std::remove_reference_t<decltype(member_ptr(T{}))>;
      constexpr auto name = hana::to<const char *>(name_c);
      const auto &member = member_ptr(entity);
      if constexpr(Meta::is_optional_v<type>)
      {
        if (member.value)
          data.emplace(name, *member.value);
        else
          data.emplace(name, nullptr);
      }
      else {
        data.emplace(name, member.value);
      }
    };
    hana::for_each(hana::accessors<T>(), hana::fuse(column_to_json));
    return data;
  }
  else {
    return {value};
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
template <typename E>
E from_json(const nlohmann::json &data)
{
  /* clang-format off */
  E entity;
  namespace hana = boost::hana;

  auto column_from_json = [&data, &entity](const auto &name_c,
                                           const auto &member_ptr) {
    auto &member = member_ptr(entity);
    using member_type = std::remove_reference_t<decltype(member)>;
    constexpr auto name = hana::to<const char *>(name_c);

    auto get_value = [&data, &name]() {
      return data[name].template get<typename member_type::type>();
    };

    if constexpr(Meta::is_primarykey_v<member_type>)
    {
      if (data.count(name) != 0 && !data[name].is_null()) {
        Sphinx::global_logger->warn("The field {}::{} is a primary key, "
                                    "value '{}' will not be deserialized",
                                    Meta::EntityName<E>, name, get_value());
      }
      return;
    }
    else if constexpr(Meta::is_optional_v<member_type> ||
                      Meta::is_foreignkey_v<member_type>)
    {
      if (data.count(name) == 0 || data[name].is_null()) {
        member.value = {};
        return;
      }
    }
    member.value = get_value();
  };

  hana::for_each(hana::accessors<E>(), hana::fuse(column_from_json));
  return entity;
  /* clang-format off */
}

} // namespace Sphinx::Json
