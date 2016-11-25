
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

template <typename T>
using EntityType = T;

template <typename Column, typename Entity>
constexpr auto is_column_belongs_to_entity =
    std::is_same_v<Entity, typename Column::entity>;

} // namespace Sphinx::Db::Meta

//----------------------------------------------------------------------
namespace Sphinx::Db {

struct just_tag {
};
struct optional_tag {
};
struct autoincrement_tag {
};

template <typename C>
constexpr bool is_optional(C &&c)
{
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

template <typename ParentTable,
          typename PK,
          typename Entity,
          auto Name,
          typename... Traits>
struct ForeignKey : public Column<Entity, typename PK::type, Name, Traits...> {

  using referenced_table = ParentTable;
  using referenced_table_primary_key = PK;

  static_assert(
      Db::Meta::is_column_belongs_to_entity<referenced_table_primary_key,
                                            referenced_table>,
      "Primary Key origin has to be ParentTable");

  Db::optional<referenced_table> referenced_value;
};

} // namespace Sphinx::Db
