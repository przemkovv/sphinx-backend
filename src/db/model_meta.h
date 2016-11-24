
#pragma once
#include <experimental/optional>
#include <type_traits>

namespace Sphinx::Db::Meta {

template <typename T>
struct Insert {
};

template <typename T>
struct ColumnsId {
};

template <typename T>
struct Table {
};

template <typename T>
constexpr auto TableName = Table<T>::name;

template <typename T>
constexpr auto InsertColumns = Insert<T>::insert_columns;

template <typename T>
using IdColumn = typename Insert<T>::id_column;


} // namespace Sphinx::Db::Meta

//----------------------------------------------------------------------
namespace Sphinx::Db {

using std::experimental::optional;
using std::experimental::nullopt;

template <typename Entity, typename Type, auto Name, bool Optional = false>
struct Column {
  using entity = Entity;
  using type = Type;
  static constexpr auto is_optional = Optional;
  static constexpr auto name = Name;

  std::conditional_t<is_optional, Db::optional<type>, type> value;
};

} // namespace Sphinx::Db
