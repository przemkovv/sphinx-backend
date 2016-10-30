
#include "db.h"
#include <sqlpp11/sqlpp11.h>

namespace Sphinx::Db {

template <typename DB>
std::vector<User> get_users(DB &db)
{
  const auto users = Tables::Users{};
  const auto total = static_cast<std::size_t>(
      db(select(count(users.id)).from(users).unconditionally()).front().count);
  auto rows = db(select(all_of(users)).from(users).unconditionally());

  std::vector<User> results;
  results.reserve(total);

  std::transform(rows.begin(), rows.end(), std::back_inserter(results),
                 [](const auto &row) { return to_user(row); });

  return results;
}

template std::vector<User> get_users(sqlpp::sqlite3::connection &db);

} // namespace Sphinx::Db
