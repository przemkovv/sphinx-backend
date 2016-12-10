
#pragma once
#include "sphinx_assert.h"
#include <array>
#include <optional>
#include <tuple>
#include <type_traits>

namespace Sphinx::Db::Meta {

template <typename T>
struct Insert {
};

template <typename T>
struct ColumnsId : public std::array<int, T::N> {
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

template <typename T>
using IdColumn_t = typename Insert<T>::id_column::type;

template <typename T>
using EntityType = T;

template <typename Column, typename Entity>
constexpr auto is_column_belongs_to_entity =
    std::is_same_v<Entity, typename Column::entity>;

template <typename Entity>
constexpr auto is_entity_v = std::is_base_of_v<Table<Entity>, Entity>;
} // namespace Sphinx::Db::Meta

//----------------------------------------------------------------------
namespace Sphinx::Db {

template <typename T, typename... Traits>
constexpr bool contains_v = false;
template <typename T, typename... Traits>
constexpr bool contains_v<T, std::tuple<Traits...>> =
    std::disjunction_v<std::is_same<T, Traits>...>;

//----------------------------------------------------------------------
struct optional_tag {
};
struct autoincrement_tag {
};
struct foreignkey_tag {
};

//----------------------------------------------------------------------
template <typename... Traits>
constexpr bool has_optional_v = contains_v<optional_tag, Traits...>;

//----------------------------------------------------------------------
template <typename C>
constexpr bool is_optional_v = contains_v<optional_tag, typename C::traits>;
template <typename C>
constexpr bool is_autoincrement_v =
    contains_v<autoincrement_tag, typename C::traits>;
template <typename C>
constexpr bool is_foreignkey_v = contains_v<foreignkey_tag, typename C::traits>;

//----------------------------------------------------------------------
template <typename C>
constexpr bool is_autoincrement(C &&)
{
  return is_autoincrement_v<typename std::remove_reference_t<C>>;
}

template <typename C>
constexpr bool is_foreignkey(C &&)
{
  return is_foreignkey_v<typename std::remove_reference_t<C>>;
}

template <typename C>
constexpr bool is_optional(C &&)
{
  return is_optional_v<typename std::remove_reference_t<C>>;
}

//----------------------------------------------------------------------

template <int N, typename Entity, typename Type, auto Name, typename... Traits>
struct Column {
  using entity = Entity;
  using type = Type;

  using traits = std::tuple<Traits...>;

  static constexpr auto name = Name;
  static constexpr auto n = N;
  constexpr operator int() { return n; }

  std::conditional_t<has_optional_v<traits>, std::optional<type>, type> value;
};

//----------------------------------------------------------------------
template <int N,
          typename ParentTable,
          typename PK,
          typename Entity,
          auto Name,
          typename... Traits>
struct ForeignKey : public Column<N,
                                  Entity,
                                  typename PK::type,
                                  Name,
                                  foreignkey_tag,
                                  Traits...> {

  using referenced_table = ParentTable;
  using referenced_table_primary_key = PK;

  static_assert(
      Db::Meta::is_column_belongs_to_entity<referenced_table_primary_key,
                                            referenced_table>,
      "Primary Key origin has to be ParentTable");

  std::optional<referenced_table> referenced_value;
};

//----------------------------------------------------------------------
template <template <int, typename, typename, auto, typename...> class C,
          int N,
          typename Entity,
          typename Type,
          auto Name,
          typename... Traits>
constexpr bool is_column(const C<N, Entity, Type, Name, Traits...> &c)
{
  using X = typename std::remove_cv_t<std::remove_reference_t<decltype(c)>>;
  using Y = Column<N, Entity, Type, Name, Traits...>;
  return std::is_convertible_v<X, Y>;
}

//----------------------------------------------------------------------
template <template <int, typename, typename, typename, auto, typename...>
          class C,
          int N,
          typename ParentTable,
          typename PK,
          typename Entity,
          auto Name,
          typename... Traits>
constexpr bool
is_column(const C<N, ParentTable, PK, Entity, Name, Traits...> &c)
{
  using X = typename std::remove_cv_t<std::remove_reference_t<decltype(c)>>;
  using Y =
      Column<N, Entity, typename PK::type, Name, foreignkey_tag, Traits...>;
  return std::is_convertible_v<X, Y>;
}

//----------------------------------------------------------------------
template <typename T>
constexpr bool is_column(T &&)
{
  return false;
}

//----------------------------------------------------------------------

} // namespace Sphinx::Db
