#pragma once

#include "model_column_traits.h"
#include <optional>     // for optional
#include <type_traits>  // for remove_cv_t, remove_reference_t, conditional_t
#include <utility>      // for tuple

namespace Sphinx::Db {

namespace {
template <typename Column, typename Entity>
constexpr auto is_column_belongs_to_entity =
    std::is_same_v<Entity, typename Column::entity>;
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

  static_assert(is_column_belongs_to_entity<referenced_table_primary_key,
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

} // namespace Sphinx::Db
