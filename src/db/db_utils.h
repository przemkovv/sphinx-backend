#pragma once

#include "model_meta.h"               // for ColumnsId
#include "shared_lib/logger.h"        // for assert_false
#include "shared_lib/sphinx_assert.h" // for assert_false
#include <algorithm>                  // for forward
#include <cstddef>                    // for nullptr_t
#include <fmt/format.h>               // for MemoryWriter
#include <libpq-fe.h> // for PGresult, PQgetisnull, PQgetvalue, PQfnumber
#include <optional>   // for optional
#include <stdint.h>   // for int64_t, uint64_t
#include <string>     // for basic_string, string
#include <tuple>      // for apply
#include <vector>     // for vector

#include <boost/hana/accessors.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/functional/apply.hpp>
#include <boost/hana/fuse.hpp>
#include <boost/hana/members.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/unpack.hpp>

namespace Sphinx::Db {

using ValueList = std::vector<std::optional<std::string>>;

//----------------------------------------------------------------------
template <typename T>
T convert_to(const char * /* data */);

//----------------------------------------------------------------------
template <typename T>
T get_field(PGresult *res, int row_id, int col_id)
{
  return convert_to<T>(PQgetvalue(res, row_id, col_id));
}

//----------------------------------------------------------------------
template <typename T>
std::optional<T> get_field_optional(PGresult *res, int row_id, int col_id)
{
  if (PQgetisnull(res, row_id, col_id)) {
    return {};
  }
  return {get_field<T>(res, row_id, col_id)};
}

//----------------------------------------------------------------------
template <typename T>
auto get_field_c(PGresult *res, int row_id, int col_id)
{
  /* clang-format off */
  using type = typename std::remove_reference_t<T>::type;
  if constexpr(Meta::is_optional_v<T>)
  {
    return get_field_optional<type>(res, row_id, col_id);
  }
  else {
    return get_field<type>(res, row_id, col_id);
  }
  /* clang-format on */
}

//----------------------------------------------------------------------
template <typename Entity, typename T>
void load_field_from_res(T &field,
                         PGresult *res,
                         int row_id,
                         const Meta::ColumnsId<Entity> &cols_id)
{
  if (cols_id[field.n] != -1)
    field.value = get_field_c<T>(res, row_id, cols_id[field.n]);
}

//----------------------------------------------------------------------
template <typename Entity, typename... Columns>
void load_fields_from_res(PGresult *res,
                          int row_id,
                          const Meta::ColumnsId<Entity> &cols_id,
                          Columns &... cols)
{
  (load_field_from_res(cols, res, row_id, cols_id), ...);
}

//----------------------------------------------------------------------
template <typename T>
T convert_to(const char * /* data */)
{
  static_assert(assert_false<T>::value, "Not implemented");
}
template <>
bool convert_to(const char *data);
template <>
int32_t convert_to(const char *data);
template <>
int64_t convert_to(const char *data);
template <>
uint64_t convert_to(const char *data);
template <>
std::string convert_to(const char *data);

//----------------------------------------------------------------------
template <typename T>
std::optional<std::string> to_optional_string(T data)
{
  fmt::MemoryWriter w;
  w.write("{}", data);
  return {w.str()};
}

//----------------------------------------------------------------------
template <typename T>
std::optional<std::string> to_optional_string(std::optional<T> data)
{
  if (data) {
    return to_optional_string(data.value());
  }
  else {
    return {};
  }
}

//----------------------------------------------------------------------
template <>
std::optional<std::string> to_optional_string(std::nullptr_t /* null */);

//----------------------------------------------------------------------
template <typename Col, typename IDs>
void get_column_id(PGresult *res, const Col &, IDs &ids)
{
  ids[Col::n] = PQfnumber(res, Col::name);
}

//----------------------------------------------------------------------
template <typename T>
Meta::ColumnsId<T> get_columns_id(PGresult *res)
{
  namespace hana = boost::hana;

  auto column_to_id = [&res](const auto &column) {
    constexpr auto name = hana::to<const char *>(hana::first(column));
    return PQfnumber(res, name);
  };
  auto ids_to_array = [](auto... ids) { return Meta::ColumnsId<T>{ids...}; };

  return hana::unpack(hana::transform(hana::accessors<T>(), column_to_id),
                      ids_to_array);
}

//----------------------------------------------------------------------
template <typename T>
T get_row(PGresult *res, const Meta::ColumnsId<T> &cols_id, int row_id)
{
  namespace hana = boost::hana;
  T entity{};
  auto func = [&res, &row_id, &cols_id](auto &&col) {
    load_field_from_res(col, res, row_id, cols_id);
  };

  hana::for_each(hana::accessors<T>(),
                 hana::fuse([&func, &entity](auto, auto member_ptr) {
                   func(member_ptr(entity));
                 }));
  return entity;
}

//----------------------------------------------------------------------
template <typename... Args>
ValueList make_value_list(Args... args)
{
  return {to_optional_string(args)...};
}
//----------------------------------------------------------------------
template <template <typename...> typename Container, typename... Args>
ValueList make_value_list(Container<Args...> &&args)
{
  return std::apply(make_value_list<Args...>,
                    std::forward<Container<Args...>>(args));
}

} // namespace Sphinx::Db
