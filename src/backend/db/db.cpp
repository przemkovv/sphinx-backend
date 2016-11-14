
#include "db.h"
#include "row_to_data.h"

#include "json_serializer.h"

namespace Sphinx::Db {

Db::Db(const connection_config &db_config)
  : db_config_(db_config), db_(db_config_)
{
}

nlohmann::json Db::get_users_json()
{
  auto query_results = get_users_query();
  auto& rows = std::get<1>(query_results);

  return rows_to_json(rows);
}

std::vector<User> Db::get_users()
{

  auto query_results = get_users_query();
  auto& total = std::get<0>(query_results);
  auto& rows = std::get<1>(query_results);

  std::vector<User> results;
  results.reserve(total);

  std::transform(rows.begin(), rows.end(), std::back_inserter(results),
                 [](const auto &row) { return to_user(row); });

  return results;
}

void Db::create_user(const User & /* user */)
{
  // const auto users = Tables::Users{};
}

} // namespace Sphinx::Db
