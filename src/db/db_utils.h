#pragma once

#include "model_meta.h"
#include "sphinx_assert.h"
#include "utils.h"

#include <fmt/format.h>

#include <libpq-fe.h>

namespace Sphinx::Db {

using std::experimental::optional;
using QueryParams = std::vector<optional<std::string>>;

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
optional<T> get_field_optional(PGresult *res, int row_id, int col_id)
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
  if
    constexpr(T::is_optional)
    {
      return get_field_optional<typename T::type>(res, row_id, col_id);
    }
  else {
    return get_field<typename T::type>(res, row_id, col_id);
  }
}

//----------------------------------------------------------------------
template <typename T>
void load_field_from_res(T &field, PGresult *res, int row_id, int col_id)
{
  field.value = get_field_c<T>(res, row_id, col_id);
}

//----------------------------------------------------------------------
template <typename T>
T convert_to(const char * /* data */)
{
  static_assert(assert_false<T>::value, "Not implemented");
}
template <>
int64_t convert_to(const char *data);
template <>
std::string convert_to(const char *data);

//----------------------------------------------------------------------
template <typename T>
optional<std::string> to_optional_string(T data)
{
  fmt::MemoryWriter w;
  w.write("{}", data);
  return {w.str()};
}

//----------------------------------------------------------------------
template <typename T>
optional<std::string> to_optional_string(optional<T> data)
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
optional<std::string> to_optional_string(std::nullptr_t /* null */);

//----------------------------------------------------------------------
template <typename T>
Meta::ColumnsId<T> get_columns_id(PGresult * /* res */)
{
  static_assert(assert_false<T>::value, "Not implemented");
}

//----------------------------------------------------------------------
template <typename T>
T get_row(PGresult * /* res */,
          const Meta::ColumnsId<T> & /*cols_id*/,
          int /*row_id*/)
{
  static_assert(assert_false<T>::value, "Not implemented");
}

//----------------------------------------------------------------------
template <typename T>
QueryParams to_insert_params(const T &data)
{
  static_assert(assert_false<T>::value, "Not implemented");
}

//----------------------------------------------------------------------
template <typename... Args>
std::vector<optional<std::string>> make_value_list(Args... args)
{
  return {to_optional_string(args)...};
}

} // namespace Sphinx::Db
