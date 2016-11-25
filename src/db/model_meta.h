
#pragma once
#include <experimental/optional>
#include <experimental/tuple>
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
constexpr auto InsertColumns = Insert<T>::columns;

template <typename T>
using IdColumn = typename Insert<T>::id_column;

} // namespace Sphinx::Db::Meta

//----------------------------------------------------------------------
namespace Sphinx::Db {

struct just_tag {
};
struct optional_tag {
};
struct autoincrement_tag {
};


template<typename C>
constexpr bool is_optional(C&& c) {
  return c.is_optional;
}

template <typename T, typename... Traits>
constexpr bool contains_v = false;
template <typename T, typename... Traits>
constexpr bool contains_v<T, std::tuple<Traits...>> =
    std::disjunction_v<std::is_same<T, Traits>...>;

using std::experimental::optional;
using std::experimental::nullopt;

template <typename Entity, typename Type, auto Name, typename... Traits>
struct Column {
  using entity = Entity;
  using type = Type;

  using traits = std::tuple<Traits...>;
  static constexpr auto is_optional = contains_v<optional_tag, traits>;
  static constexpr auto is_autoincrement =
      contains_v<autoincrement_tag, traits>;
  static constexpr auto name = Name;

  std::conditional_t<is_optional, Db::optional<type>, type> value;
};

} // namespace Sphinx::Db
