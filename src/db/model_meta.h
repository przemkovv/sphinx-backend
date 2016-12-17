#pragma once

#include "model_column.h"
#include "model_column_traits.h" // for is_primarykey
#include <array>                 // for array
#include <tuple>                 // for make_tuple
#include <type_traits>           // for remove_reference_t, negation
#include <vector>                // for vector

#include <boost/hana/accessors.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/filter.hpp>
#include <boost/hana/find_if.hpp>
#include <boost/hana/fuse.hpp>
#include <boost/hana/integral_constant.hpp>
#include <boost/hana/members.hpp>
#include <boost/hana/optional.hpp>
#include <boost/hana/unpack.hpp>

namespace Sphinx::Db::Meta {

//----------------------------------------------------------------------
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

template <typename E>
constexpr auto is_entity_v = std::is_base_of_v<Entity<E>, E>;

//----------------------------------------------------------------------
template <typename Entity>
constexpr auto get_insert_columns()
{
  namespace hana = boost::hana;
  auto is_not_pk = [](auto member_pair) {
    auto &member_ptr = hana::second(member_pair);
    using type = std::remove_reference_t<decltype(member_ptr(Entity{}))>;
    return std::negation<is_primarykey<type>>{};
  };
  return hana::filter(hana::accessors<Entity>(), is_not_pk);
}
//----------------------------------------------------------------------
template <typename Entity>
constexpr auto get_values_to_insert(Entity &&entity)
{
  namespace hana = boost::hana;
  auto is_not_pk = [](auto &&column) {
    using type = std::remove_reference_t<decltype(column)>;
    return std::negation<is_primarykey<type>>{};
  };
  auto to_tuple = [](auto &&... columns) {
    auto value_of = [](auto &&column) { return column.value; };
    return std::make_tuple(value_of(columns)...);
  };

  return hana::unpack(hana::filter(hana::members(entity), is_not_pk), to_tuple);
}

//----------------------------------------------------------------------
template <typename Entity>
constexpr auto get_id_column_hana_pair()
{
  namespace hana = boost::hana;
  auto is_pk = [](auto column) {
    auto &member_ptr = hana::second(column);
    using type = std::remove_reference_t<decltype(member_ptr(Entity{}))>;
    return is_primarykey<type>{};
  };
  auto pk = hana::find_if(hana::accessors<Entity>(), is_pk);
  static_assert(!hana::is_nothing(pk), "Could not find primary key");
  return pk.value();
}

//----------------------------------------------------------------------
template <typename Entity>
constexpr auto get_id_column_ptr()
{
  namespace hana = boost::hana;
  auto pk = get_id_column_hana_pair<Entity>();
  return hana::second(pk);
}

//----------------------------------------------------------------------
template <typename Entity>
constexpr auto get_id_column_name()
{
  namespace hana = boost::hana;
  auto pk = get_id_column_hana_pair<Entity>();
  return hana::to<const char *>(hana::first(pk));
}

//----------------------------------------------------------------------
template <typename Entity, int N>
constexpr auto get_nth_column_hana_pair()
{
  namespace hana = boost::hana;
  auto m = hana::find_if(
      hana::accessors<Entity>(), hana::fuse([](auto, auto member_ptr) {
        using type = std::remove_reference_t<decltype(member_ptr(Entity{}))>;
        return hana::bool_c<hana::equal(type::n, N)>;
      }));
  static_assert(!hana::is_nothing(m), "Could not find member");
  return m.value();
}

//----------------------------------------------------------------------
template <typename Entity, int N>
constexpr auto get_nth_column_ptr()
{
  namespace hana = boost::hana;
  auto m = get_nth_column_hana_pair<Entity, N>();
  return hana::second(m);
}

//----------------------------------------------------------------------
template <typename Entity, int N>
constexpr auto get_nth_column_name()
{
  namespace hana = boost::hana;
  auto m = get_nth_column_hana_pair<Entity, N>();
  return hana::to<const char *>(hana::first(m));
}

//----------------------------------------------------------------------
template <typename Column>
constexpr auto get_column_name()
{
  namespace hana = boost::hana;
  using Entity = typename Column::entity;
  return get_nth_column_name<Entity, Column::n>();
}

//----------------------------------------------------------------------
template <typename T>
using IdColumn = std::remove_reference_t<decltype(get_id_column_ptr<T>()(T{}))>;

template <typename Entity>
using IdColumnType = typename IdColumn<Entity>::type;

template <typename Entity>
constexpr auto IdColumnName = get_id_column_name<Entity>();

//----------------------------------------------------------------------

} // namespace Sphinx::Db::Meta
