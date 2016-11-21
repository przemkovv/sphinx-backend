
#pragma once
#include <experimental/optional>

namespace Sphinx::Db::Meta {

//----------------------------------------------------------------------
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
