
#pragma once
#include <experimental/optional>

namespace Sphinx::Db::Meta {

template <typename Entity, typename Type, auto Name>
struct Column {
  using entity = Entity;
  using type = Type;
  static constexpr auto name = Name;

  Type value;
};

template <typename Column>
struct type {
  using value = typename Column::type;
};

template <typename Column>
struct name {
  static constexpr auto value = Column::name;
  name(const Column& c){}
};
// template <typename Column>
// constexpr auto name(const Column &[> c <]) {
  // return Column::name;
// }

template <typename T>
struct Columns {
};

template <typename T>
struct ColumnsId {
};

template <typename T>
struct Table {
};

} // namespace Sphinx::Db::Meta

//----------------------------------------------------------------------
namespace Sphinx::Db {

using std::experimental::optional;
using std::experimental::nullopt;

template <typename T>
struct TableDescription {
  using Columns = Meta::Columns<T>;
  using ColumnsId = Meta::ColumnsId<T>;
  using Table = Meta::Table<T>;
};

} // namespace Sphinx::Db
