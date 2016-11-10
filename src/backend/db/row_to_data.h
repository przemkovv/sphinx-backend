
#pragma once

#include "data.h"

namespace Sphinx::Db {

template <typename UserRow>
User to_user(const UserRow &user_row)
{
  return {user_row.id, user_row.username, user_row.email};
}

} // namespace Sphinx::Db
