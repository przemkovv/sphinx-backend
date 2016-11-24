#pragma once

#include "Logger.h"              // for Logger
#include "db_utils.h"            // for make_value_list, ValueList, get_c...
#include "model_meta.h"          // for Columns
#include "sphinx_assert.h"       // for SPHINX_ASSERT
#include <algorithm>             // for move
#include <cstddef>               // for size_t
#include <exception>             // for exception
#include <experimental/optional> // for optional
#include <fmt/format.h>          // for format, UdlArg, operator""_a
#include <functional>            // for function
#include <iterator>              // for next
#include <libpq-fe.h>            // for PQerrorMessage, PGresult, PQresultS...
#include <memory>                // for shared_ptr, __shared_ptr_access
#include <numeric>               // for accumulate
#include <spdlog/spdlog.h>       // for logger
#include <stdexcept>             // for runtime_error
#include <stdint.h>              // for uint16_t
#include <string>                // for string
#include <vector>                // for vector

namespace Sphinx::Db {

using std::experimental::optional;
using fmt::literals::operator""_a;

//----------------------------------------------------------------------
struct connection_config {
  std::string host;
  std::uint16_t port;
  std::string user;
  std::string password;
  std::string db_name;

  std::string get_connection_string() const
  {
    return fmt::format("postgresql://{user}:{password}@{host}:{port}/{db_name}",
                       "user"_a = user, "password"_a = password,
                       "host"_a = host, "port"_a = port, "db_name"_a = db_name);
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
  auto exec(const std::string &query,
            const std::vector<optional<std::string>> &args)
  {
    auto pq_args = make_pq_args(args);

    logger_->info("Executing query: {}", query);

    auto result = PQexecParams(conn_.get(), query.data(),
                               static_cast<int>(pq_args.size()), nullptr,
                               pq_args.data(), nullptr, nullptr, 0);

    return make_result_safe(result);
  }
  //----------------------------------------------------------------------
  template <typename... Args>
  auto exec(const std::string &query, Args... param_args)
  {
    auto args = make_value_list(param_args...);
    return exec(query, args);
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
        exec(fmt::format("SELECT * FROM {0}", std::string{Meta::TableName<T>}));
    return get_rows<T>(std::move(result));
  }

  //----------------------------------------------------------------------
  template <typename T>
  std::string prepare_insert_query()
  {
    auto &insert_columns = Meta::Insert<T>::columns;

    std::string field_list =
        std::accumulate(std::next(insert_columns.begin()), insert_columns.end(),
                        std::string(*insert_columns.begin()),
                        [](auto a, auto b) { return a + ", " + b; });

    int n = 1;
    std::string field_ids =
        std::accumulate(std::next(insert_columns.begin()), insert_columns.end(),
                        std::string{"$1"}, [n](auto a, auto b) mutable {
                          return a + ", " + fmt::format("${}", ++n);
                        });

    std::string table_name = Meta::TableName<T>;
    std::string id_column = Meta::IdColumn<T>::name;
    auto query = fmt::format("INSERT INTO {0} ({1}) VALUES ({2}) RETURNING {3}",
                             table_name, field_list, field_ids, id_column);
    return query;
  }

  //----------------------------------------------------------------------
  template <typename T>
  auto insert(const T &data)
  {
    auto query = prepare_insert_query<T>();
    auto insert_params = make_value_list(Meta::Insert<T>::values(data));
    return insert<T>(query, insert_params);
  }

  //----------------------------------------------------------------------
  template <typename T>
  typename Meta::IdColumn<T>::type insert(const std::string &query,
                                          const ValueList &insert_params)
  {
    auto res = exec(query, insert_params);

    int status = PQresultStatus(res.get());
    if (status == PGRES_COMMAND_OK) {
      return {};
    }
    if (status == PGRES_TUPLES_OK) {

      using id_column_t = typename Meta::IdColumn<T>::type;
      auto id_column = Meta::IdColumn<T>::name;

      auto id_column_id = PQfnumber(res.get(), id_column);
      int row_count = PQntuples(res.get());

      SPHINX_ASSERT(row_count == 1, "INSERT query returned more than one row");

      auto last_id = get_field<id_column_t>(res.get(), 0, id_column_id);
      return last_id;
    }

    logger_->error(PQerrorMessage(conn_.get()));
    throw std::runtime_error(PQerrorMessage(conn_.get()));
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

protected:
  Logger logger_;
};

} // namespace Sphinx::Db

//----------------------------------------------------------------------
