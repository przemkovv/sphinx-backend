#pragma once

#include "sphinx_assert.h"
#include <array>
#include <optional>
#include <tuple>
#include <type_traits>
#include <vector>

#include <boost/hana/integral_constant.hpp>
#include <boost/hana/accessors.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/filter.hpp>
#include <boost/hana/find_if.hpp>
#include <boost/hana/fuse.hpp>
#include <boost/hana/members.hpp>
#include <boost/hana/unpack.hpp>
#include <boost/hana/value.hpp>

namespace Sphinx::Db::Meta {

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

template <int N, typename Entity, typename Type, typename... Traits>
struct Column {
  using entity = Entity;
  using type = Type;

  using traits = std::tuple<Traits...>;

  static constexpr auto n = N;

  std::conditional_t<has_optional_v<traits>, std::optional<type>, type> value;
};

//----------------------------------------------------------------------
template <int N,
          typename ParentTable,
          typename PK,
          typename Entity,
          typename... Traits>
struct ForeignKey
    : public Column<N, Entity, typename PK::type, foreignkey_tag, Traits...> {

  using referenced_table = ParentTable;
  using referenced_table_primary_key = PK;

  static_assert(Meta::is_column_belongs_to_entity<referenced_table_primary_key,
                                                  referenced_table>,
                "Primary Key origin has to be ParentTable");

  std::optional<referenced_table> referenced_value;
};

//----------------------------------------------------------------------
template <template <int, typename, typename, typename...> class C,
          int N,
          typename Entity,
          typename Type,
          typename... Traits>
constexpr bool is_column(const C<N, Entity, Type, Traits...> &c)
{
  using X = typename std::remove_cv_t<std::remove_reference_t<decltype(c)>>;
  using Y = Column<N, Entity, Type, Traits...>;
  return std::is_convertible_v<X, Y>;
}

//----------------------------------------------------------------------
template <template <int, typename, typename, typename, typename...> class C,
          int N,
          typename ParentTable,
          typename PK,
          typename Entity,
          typename... Traits>
constexpr bool is_column(const C<N, ParentTable, PK, Entity, Traits...> &c)
{
  using X = typename std::remove_cv_t<std::remove_reference_t<decltype(c)>>;
  using Y = Column<N, Entity, typename PK::type, foreignkey_tag, Traits...>;
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

//----------------------------------------------------------------------
template <typename Entity>
constexpr auto get_insert_columns()
{
  namespace hana = boost::hana;
  return hana::filter(hana::accessors<Entity>(), [](auto column) {
    auto &c = hana::second(column);
    using type = std::remove_reference_t<decltype((c(Entity{})))>;
    return std::negation<is_primarykey<type>>{};
  });
}
//----------------------------------------------------------------------
template <typename Entity>
constexpr auto get_values_to_insert(Entity &&entity)
{
  namespace hana = boost::hana;
  return hana::unpack(
      hana::filter(hana::members(entity),
                   [](auto &&column) {
                     using type = std::remove_reference_t<decltype(column)>;
                     return std::negation<is_primarykey<type>>{};
                   }),
      [](auto &&... columns) {
        auto value_of = [](auto &&column) { return column.value; };
        return std::make_tuple(value_of(columns)...);
      });
}
//----------------------------------------------------------------------
template <typename Entity>
constexpr auto get_hana_id_column()
{
  namespace hana = boost::hana;
  return hana::find_if(hana::accessors<Entity>(),
                       [](auto column) {
                         auto &c = hana::second(column);
                         using type =
                             std::remove_reference_t<decltype(c(Entity{}))>;
                         return is_primarykey<type>{};
                       })
      .value();
}

template <typename Entity>
constexpr auto get_id_column()
{
  namespace hana = boost::hana;
  return hana::second(get_hana_id_column<Entity>());
}

template <typename Entity>
constexpr auto get_id_column_name()
{
  namespace hana = boost::hana;
  return hana::to<const char *>(hana::first(get_hana_id_column<Entity>()));
}

template <typename Entity, auto N>
constexpr auto get_nth_column_ptr(Entity &&entity)
{
  namespace hana = boost::hana;
  constexpr auto n = N;
  return hana::find_if(
               hana::members(entity), [](auto &&member) {
                 using type =
                     std::remove_reference_t<decltype(member(Entity{}))>;
                 return hana::bool_c<hana::equal(type::n , n)>;
                 }) ;
}

template <typename Entity, auto N>
constexpr auto get_nth_column_name()
{
  namespace hana = boost::hana;
  auto m = hana::find_if(
               hana::accessors<Entity>(), hana::fuse([](auto, auto member) {
                 using type =
                     std::remove_reference_t<decltype(member(Entity{}))>;
                 return hana::bool_c<hana::equal(type::n , N)>;
                 }))
               .value();
  return hana::to<const char *>(hana::first(m));
}

template <typename Column>
constexpr auto get_column_name()
{
  namespace hana = boost::hana;
  using Entity = typename Column::entity;
  // TODO: calling this function causes segmentation fault of the GCC compiler
  // return get_nth_column_name<Entity, Column::n>(); 
  auto m = hana::find_if(
               hana::accessors<Entity>(), hana::fuse([](auto, auto member) {
                 using type =
                     std::remove_reference_t<decltype(member(Entity{}))>;
                 return hana::bool_c<hana::equal(type::n , Column::n)>;
                 }))
               .value();

  return hana::to<const char *>(hana::first(m));
}

} // namespace Sphinx::Db

namespace Sphinx::Db::Meta {

template <typename T>
using IdColumn = std::remove_reference_t<decltype(get_id_column<T>()(T{}))>;

template <typename Entity>
using IdColumnType = typename IdColumn<Entity>::type;

template <typename Entity>
constexpr auto IdColumnName = get_id_column_name<Entity>();

} // namespace Sphinx::Db::Meta
