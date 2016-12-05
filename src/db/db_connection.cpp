
#include "db_connection.h"
#include <algorithm>   // for move
#include <type_traits> // for remove_reference<>::type

namespace Sphinx::Db {

//----------------------------------------------------------------------
std::shared_ptr<PGconn>
DbConnection::connect(const connection_config &db_config)
{
  return {PQconnectdb(db_config.get_connection_string().data()),
          [&](PGconn *conn) {
            logger_->info("Closing database connection");
            if (PQstatus(conn_.get()) == CONNECTION_OK) {
              PQfinish(conn);
            }
          }};
}

//----------------------------------------------------------------------
DbConnection::DbConnection(connection_config db_config)
  : db_config_(std::move(db_config)),
    conn_(connect(db_config_)),
    logger_(Sphinx::make_logger("Sphinx::DbConnection", spdlog::level::debug))
{
  if (PQstatus(conn_.get()) != CONNECTION_OK) {
    logger_->error("Cannot connect to database: {}",
                   PQerrorMessage(conn_.get()));
  }
  else {
    logger_->info("Connected to database.");
  }
}

//----------------------------------------------------------------------
auto DbConnection::make_result_safe(PGresult *res)
    -> std::unique_ptr<PGresult, std::function<void(PGresult *)>>
{
  return std::unique_ptr<PGresult, std::function<void(PGresult *)>>(
      res, [=](PGresult *res_) { PQclear(res_); });
}

//----------------------------------------------------------------------
prepared_statement DbConnection::prepare(const std::string &name,
                                         const std::string &query,
                                         int parameters)
{
  auto tmp =
      PQprepare(conn_.get(), name.c_str(), query.c_str(), parameters, nullptr);
  auto res = make_result_safe(tmp);

  if (PQresultStatus(res.get()) == PGRES_COMMAND_OK) {
    return prepared_statement(name, parameters);
  }
  auto msg = PQerrorMessage(conn_.get());
  logger_->error("{}", msg);
  throw std::runtime_error(msg);
}

//----------------------------------------------------------------------
std::vector<const char *> DbConnection::make_pq_args(const ValueList &arguments)
{
  std::vector<const char *> pq_args;

  for (auto &val : arguments) {
    if (!val) {
      pq_args.push_back(nullptr);
    }
    else {
      pq_args.push_back(val.value().data());
    }
  }

  return pq_args;
}

//----------------------------------------------------------------------
} // namespace Sphinx::Db
