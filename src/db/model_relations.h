#pragma once

#include "model_meta.h"
#include "shared_lib/sphinx_assert.h"
#include <optional>
#include <type_traits>

#include <boost/hana/fwd/second.hpp>

namespace Sphinx::Db {

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

namespace Sphinx::Db::Meta {

//----------------------------------------------------------------------
template <typename Link>
constexpr auto get_remote_key_member_ptr()
{
  namespace hana = boost::hana;
  using RemoteKey = typename Link::remote_key;
  using RemoteEntity = typename Link::remote_entity;
  auto m = get_nth_column_hana_pair<RemoteEntity, RemoteKey::n>();
  return hana::second(m);
}

} // namespace Sphinx::Db::Meta
