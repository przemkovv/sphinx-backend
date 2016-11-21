#pragma once

#include "db_utils.h"
#include "model.h"
#include "model_utils.h"

#include "Logger.h"
#include "utils.h"
#include <fmt/format.h>
#include <libpq-fe.h>
#include <nlohmann/json.hpp>

#include <vector>

#include <experimental/optional>
#include <experimental/propagate_const>
#include <memory>

#include "sphinx_assert.h"

namespace Sphinx::Db {

using std::experimental::optional;

//----------------------------------------------------------------------
struct connection_config {
  std::string host;
  std::uint16_t port;
  std::string user;
  std::string password;
  std::string db_name;

  std::string get_connection_string() const
  {
    return fmt::format("postgresql://{0}:{1}@{2}:{3}/{4}", user, password, host,
                       port, db_name);
  }
};

//----------------------------------------------------------------------
class prepared_statement {
public:
  prepared_statement(const std::string &name, int parameters)
    : name_(name), parameters_(parameters)
  {
  }

  const std::string &get_name() const { return name_; }
  int get_parameters() const { return parameters_; }

private:
  std::string name_;
  int parameters_;
};

//----------------------------------------------------------------------
class DbConnection {

public:
  DbConnection(connection_config db_config);
  ~DbConnection() = default;

private:
  /* database connection and statement execution */
  const connection_config db_config_;

  std::shared_ptr<PGconn> conn_;
  std::shared_ptr<PGconn> connect(const connection_config &db_config);

  auto make_result_safe(PGresult *res)
      -> std::unique_ptr<PGresult, std::function<void(PGresult *)>>;

public:
  //----------------------------------------------------------------------
  template <typename... Args>
  auto exec(const std::string &query, Args... param_args)
  {
    auto args = make_value_list(param_args...);
    auto pq_args = make_pq_args(args);

    logger_->info("Executing query: {}", query);

    auto result = PQexecParams(conn_.get(), query.data(),
                               static_cast<int>(pq_args.size()), nullptr,
                               pq_args.data(), nullptr, nullptr, 0);

    return make_result_safe(result);
  }

  //----------------------------------------------------------------------
  template <typename... Args>
  auto exec(const prepared_statement &stmt, Args... param_args)
  {
    auto args = make_value_list(param_args...);
    auto pq_args = make_pq_args(args);

    logger_->info("Executing prepared query: {}", stmt.get_name());

    auto result = PQexecPrepared(conn_.get(), stmt.get_name().c_str(),
                                 stmt.get_parameters(), pq_args.data(), nullptr,
                                 nullptr, 0);

    return make_result_safe(result);
  }

  prepared_statement
  prepare(const std::string &name, const std::string &query, int parameters);

public:
  //----------------------------------------------------------------------
  template <typename T>
  std::vector<T> get_all()
  {
    auto result =
        exec(fmt::format("SELECT * FROM {0}", std::string{T::Table::name}));
    return get_rows<T>(std::move(result));
  }

  //----------------------------------------------------------------------
  template <typename T, typename Res>
  std::vector<T> get_rows(Res &&res)
  {
    int status = PQresultStatus(res.get());

    if (status == PGRES_COMMAND_OK) {
      return {};
    }
    if (status == PGRES_TUPLES_OK) {

      const auto cols = get_columns_id<T>(res.get());
      int row_count = PQntuples(res.get());

      std::vector<T> rows;
      rows.reserve(static_cast<std::size_t>(row_count));

      for (int r = 0; r < row_count; r++) {
        rows.push_back(get_row<T>(res.get(), cols, r));
      }
      return rows;
    }

    logger_->error(PQerrorMessage(conn_.get()));
    throw std::runtime_error(PQerrorMessage(conn_.get()));
  }

  //----------------------------------------------------------------------
  std::vector<const char *>
  make_pq_args(const std::vector<optional<std::string>> &arguments);

  //----------------------------------------------------------------------
  template <typename... Args>
  std::vector<optional<std::string>> make_value_list(Args... args)
  {
    return {to_optional_string(args)...};
  }

  //----------------------------------------------------------------------

private:
  Logger logger_;
};

} // namespace Sphinx::Db

//----------------------------------------------------------------------
