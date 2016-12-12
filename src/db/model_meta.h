#pragma once

#include "sphinx_assert.h"
#include <array>
#include <optional>
#include <tuple>
#include <type_traits>
#include <vector>

namespace Sphinx::Db::Meta {

template <typename T>
struct Insert {
};

template <typename T>
struct ColumnsId : public std::array<int, T::N> {
};

template <typename T>
struct Entity {
};

template <typename E>
using Entities = std::vector<E>;

template <typename T>
constexpr auto EntityName = Entity<T>::name;

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

template <typename E>
constexpr auto is_entity_v = std::is_base_of_v<Entity<E>, E>;
} // namespace Sphinx::Db::Meta

//----------------------------------------------------------------------
namespace Sphinx::Db {
template <typename T, typename... Traits>
struct contains : std::false_type {
};
template <typename T, typename... Traits>
struct contains<T, std::tuple<Traits...>>
    : std::disjunction<std::is_same<T, Traits>...> {
};

template <typename T, typename... Traits>
constexpr bool contains_v = contains<T, Traits...>::value;

//----------------------------------------------------------------------
struct primarykey_tag {
};
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
struct is_primarykey : contains<primarykey_tag, typename C::traits> {
};
template <typename C>
struct is_optional : contains<optional_tag, typename C::traits> {
};
template <typename C>
struct is_autoincrement : contains<autoincrement_tag, typename C::traits> {
};
template <typename C>
struct is_foreignkey : contains<foreignkey_tag, typename C::traits> {
};

template <typename C>
constexpr bool is_primarykey_v = is_primarykey<C>::value;
template <typename C>
constexpr bool is_optional_v = is_optional<C>::value;
template <typename C>
constexpr bool is_autoincrement_v = is_autoincrement<C>::value;
template <typename C>
constexpr bool is_foreignkey_v = is_foreignkey<C>::value;

//----------------------------------------------------------------------
template <typename C>
constexpr bool is_autoincrement_c(C &&)
{
  return is_autoincrement_v<typename std::remove_reference_t<C>>;
}

template <typename C>
constexpr bool is_foreignkey_c(C &&)
{
  return is_foreignkey_v<typename std::remove_reference_t<C>>;
}

template <typename C>
constexpr bool is_primarykey_c(C &&)
{
  return is_primarykey_v<typename std::remove_reference_t<C>>;
}
template <typename C>
constexpr bool is_optional_c(C &&)
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

  static_assert(Meta::is_column_belongs_to_entity<referenced_table_primary_key,
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

//----------------------------------------------------------------------

template <typename T>
using LinkManyFieldType = std::optional<T>;

//----------------------------------------------------------------------
template <typename T, typename RemoteKey, typename LocalKey, auto Name>
struct LinkManyInfo {

  using type = T;
  using remote_key = RemoteKey;
  using remote_entity = typename RemoteKey::entity;
  using local_key = LocalKey;
  using local_entity = typename LocalKey::entity;
  static constexpr auto name = Name;
};

struct LinkOne {
};

template <typename LocalMember>
struct LinkManyType {
  typedef typename LocalMember::a b;
  static_assert(assert_false<LocalMember>::value, "Not implemented");
};

//----------------------------------------------------------------------
template <typename LocalMember>
using LinkMany =
    typename LinkManyType<std::remove_reference_t<LocalMember>>::type;

} // namespace Sphinx::Db
