#pragma once

#include "model_utils.h"
#include "sphinx_assert.h"
#include "utils.h"

#include <libpq-fe.h>

namespace Sphinx::Db {

using std::experimental::optional;

template <typename T>
T get_row(PGresult * /* res */,
          const Sphinx::Db::Meta::ColumnsId<T> & /*cols*/,
          int /*row_id*/);

template <typename T>
Sphinx::Db::Meta::ColumnsId<T> get_columns_id(PGresult * /* res */);

template <typename T>
T get_field(PGresult *res, int row_id, int col_id);

template <typename T>
optional<T> get_field_optional(PGresult *res, int row_id, int col_id);

template <typename T>
auto get_field_c(PGresult *res, int row_id, int col_id)
{
  if
    constexpr(Utils::is_optional<T>::value)
    {
      return get_field_optional<typename Utils::remove_optional<T>::type>(
          res, row_id, col_id);
    }
  else {
    return get_field<T>(res, row_id, col_id);
  }
}

template <typename T>
void load_field_from_res(T &field, PGresult *res, int row_id, int col_id)
{
  field = get_field_c<T>(res, row_id, col_id);
}
template <typename T>
T convert_to(const char * /* data */);

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
  return {std::to_string(data)};
}
template <>
optional<std::string> to_optional_string(const char *data);
template <>
optional<std::string> to_optional_string(const std::string &data);
template <>
optional<std::string> to_optional_string(std::nullptr_t /* null */);

//----------------------------------------------------------------------
template <typename T>
Meta::ColumnsId<T> get_columns_id(PGresult * /* res */)
{
  static_assert(assert_false<T>::value, "Not implemented");
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
template <typename T>
T get_row(PGresult * /* res */,
          const Meta::ColumnsId<T> & /*cols_id*/,
          int /*row_id*/)
{
  static_assert(assert_false<T>::value, "Not implemented");
}

} // namespace Sphinx::Db
