
#include "db.h"
#include "row_to_data.h"

#include "json_serializer.h"

#include <algorithm>
#include <iterator>
#include <utility>

namespace Sphinx::Db {

// struct Db::Queries {

  // Queries(connection &db) : db_(db) {}

  // auto get_users() const
  // {
    // const auto users = Tables::Users{};
    // const auto total = static_cast<std::size_t>(
        // db_(select(sqlpp::count(users.id)).from(users).unconditionally())
            // .front()
            // .count);
    // auto rows = db_(sqlpp::select(all_of(users)).from(users).unconditionally());
    // return std::make_tuple(total, std::move(rows));
  // }

  // auto create_user(const User &user) const
  // {
    // const auto users = Tables::Users{};
    // std::size_t id = 0;
    // id = db_(insert_into(users).set(users.username = user.username,
                                    // users.email = user.email));

    // return std::make_tuple(id, SqlStatusCode::OK);
  // }

// private:
  // connection &db_;
/* } */;

Db::Db(const connection_config &db_config)
  : db_config_(db_config), logger_(Sphinx::make_logger("Sphinx::Db", spdlog::level::debug))
{
  logger_->info("Connecting to database");
  conn = PQconnectdb(db_config_.get_connection_string().data());
  if (PQstatus(conn) != CONNECTION_OK) {
    logger_->error("Cannot connect to database: {}", PQerrorMessage(conn));
  } else {
    logger_->info("Connected to database.");
  }
}

Db::~Db() {
  if (PQstatus(conn) == CONNECTION_OK){
    logger_->info("Database disconnecting");
    PQfinish(conn);
  }
}

// nlohmann::json Db::get_users_json()
// {
  // auto query_results = queries_->get_users();
  // auto &rows = std::get<1>(query_results);

  // return rows_to_json(rows);
// }

// std::vector<User> Db::get_users()
// {

  // auto query_results = queries_->get_users();
  // auto &total = std::get<0>(query_results);
  // auto &rows = std::get<1>(query_results);

  // std::vector<User> results;
  // results.reserve(total);

  // std::transform(rows.begin(), rows.end(), std::back_inserter(results),
                 // [](const auto &row) { return to_user(row); });

  // return results;
// }

// void Db::create_user(const User &user) { queries_->create_user(user); }
} // namespace Sphinx::Db
