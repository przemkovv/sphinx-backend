
#pragma once
#include <optional>
#include <tuple>
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

template <typename T>
using IdColumn_t = typename Insert<T>::id_column::type;

template <typename T>
using EntityType = T;

template <typename Column, typename Entity>
constexpr auto is_column_belongs_to_entity =
    std::is_same_v<Entity, typename Column::entity>;

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

template <typename Entity, typename Type, auto Name, typename... Traits>
struct Column {
  using entity = Entity;
  using type = Type;

  using traits = std::tuple<Traits...>;

  static constexpr auto name = Name;

  std::conditional_t<has_optional_v<traits>, std::optional<type>, type> value;
};

//----------------------------------------------------------------------
template <typename ParentTable,
          typename PK,
          typename Entity,
          auto Name,
          typename... Traits>
struct ForeignKey : public Column<Entity,
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
} // namespace Sphinx::Db
