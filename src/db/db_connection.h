#pragma once

#include "db_utils.h"                 // for ValueList, make_value_list
#include "model_meta.h"               // for IdColumnType, get_insert_columns
#include "shared_lib/logger.h"        // for Logger
#include "shared_lib/sphinx_assert.h" // for SPHINX_ASSERT

#include <boost/hana/config.hpp>      // for hana
#include <boost/hana/fwd/core/to.hpp> // for to
#include <boost/hana/fwd/first.hpp>   // for first
#include <boost/hana/fwd/fold.hpp>    // for fold
#include <cstddef>                    // for size_t
#include <exception>                  // for exception
#include <fmt/format.h>               // for operator""_a, format, UdlArg
#include <functional>                 // for function
#include <libpq-fe.h>                 // for PQerrorMessage, PQntuples, PQr...
#include <memory>                     // for shared_ptr, __shared_ptr_access
#include <numeric>                    // for accumulate
#include <optional>                   // for optional
#include <spdlog/spdlog.h>            // for logger
#include <stdexcept>                  // for runtime_error
#include <stdint.h>                   // for uint16_t
#include <string>                     // for basic_string, string
#include <type_traits>                // for remove_reference_t
#include <vector>                     // for vector

namespace Sphinx::Db {

using fmt::literals::operator""_a;

//----------------------------------------------------------------------
struct connection_config {
  std::string host;
  std::uint16_t port;
  std::string user;
  std::string password;
  std::string db_name;

  std::string init_script;

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
  auto exec_script(const std::string &query)
  {

    logger_->info("Executing query: {}", query);

    auto result = PQexec(conn_.get(), query.data());

    int status = PQresultStatus(result);
    if (status != PGRES_COMMAND_OK) {
      logger_->error(PQerrorMessage(conn_.get()));
      throw std::runtime_error(PQerrorMessage(conn_.get()));
    }

    return make_result_safe(result);
  }
  //----------------------------------------------------------------------
  auto exec(const std::string &query, const ValueList &args)
  {
    auto pq_args = make_pq_args(args);

    std::string args_str = std::accumulate(
        args.begin(), args.end(), std::string{""},
        [](auto a, auto b) { return fmt::format("{}, \"{}\"", a, *b); });

    logger_->info("Executing query: {}\nparams: {}", query, args_str);

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
    auto result = exec(fmt::format("SELECT * FROM {0}", Meta::EntityName<T>));
    return get_rows<T>(std::move(result));
  }
  //----------------------------------------------------------------------
  template <typename T, typename Condition>
  std::vector<T> get_all_where(const Condition &condition)
  {
    auto result = exec(fmt::format("SELECT * FROM {0} WHERE {1}",
                                   Meta::EntityName<T>, condition.str("$1")),
                       condition.value);
    return get_rows<T>(std::move(result));
  }
  //----------------------------------------------------------------------
  template <typename T>
  std::optional<T> find_by_id(Meta::IdColumnType<T> id)
  {
    auto result =
        exec(fmt::format("SELECT * FROM {table_name} WHERE {column} = {value}",
                         "table_name"_a = Meta::EntityName<T>,
                         "column"_a = Meta::IdColumnName<T>, "value"_a = id));
    return get_row<T>(std::move(result));
  }
  //----------------------------------------------------------------------
  template <typename T>
  T get_by_id(typename Meta::IdColumnType<T> id)
  {
    return find_by_id<T>(id).value();
  }
  //----------------------------------------------------------------------
  template <typename T>
  bool exists(typename Meta::IdColumnType<T> id)
  {
    auto result = exec(fmt::format(
        "SELECT EXISTS(SELECT 1 FROM {table_name} WHERE {column} = {value})",
        "table_name"_a = Meta::EntityName<T>,
        "column"_a = Meta::IdColumnName<T>, "value"_a = id));

    return get_scalar<bool>(std::move(result));
  }
  //----------------------------------------------------------------------
  template <typename T, typename Res>
  T get_scalar(Res &&res)
  {
    int status = PQresultStatus(res.get());

    if (status == PGRES_TUPLES_OK) {
      int row_count = PQntuples(res.get());
      SPHINX_ASSERT(row_count == 1, "Number of rows have to be 1 (is {})",
                    row_count);

      return get_field<bool>(res.get(), 0, 0);
    }

    logger_->error(PQerrorMessage(conn_.get()));
    throw std::runtime_error(PQerrorMessage(conn_.get()));
  }
  //----------------------------------------------------------------------
  template <typename T>
  std::string prepare_insert_query()
  {
    namespace hana = boost::hana;

    std::string field_list = hana::fold(
        Meta::get_insert_columns<T>(), std::string{}, [](auto a, auto b) {
          return fmt::format("{}{},", a,
                             hana::to<const char *>(hana::first(b)));
        });
    field_list.pop_back();

    int n = 0;
    std::string field_ids = hana::fold(
        Meta::get_insert_columns<T>(), std::string{},
        [n](auto a, auto) mutable { return fmt::format("{}${},", a, ++n); });
    field_ids.pop_back();

    constexpr auto table_name = Meta::EntityName<T>;
    constexpr auto id_column = Meta::IdColumnName<T>;
    auto query = fmt::format("INSERT INTO {0} ({1}) VALUES ({2}) RETURNING {3}",
                             table_name, field_list, field_ids, id_column);
    return query;
  }

  //----------------------------------------------------------------------
  template <typename T>
  auto insert(const T &data)
  {
    auto query = prepare_insert_query<T>();
    auto insert_params = make_value_list(get_values_to_insert(data));
    return insert<T>(query, insert_params);
  }

  //----------------------------------------------------------------------
  template <typename T>
  typename Meta::IdColumnType<T> insert(const std::string &query,
                                        const ValueList &insert_params)
  {
    auto res = exec(query, insert_params);

    int status = PQresultStatus(res.get());
    if (status == PGRES_COMMAND_OK) {
      return {};
    }
    if (status == PGRES_TUPLES_OK) {

      using id_column_t = typename Meta::IdColumnType<T>;
      constexpr auto id_column = Meta::IdColumnName<T>;

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
  std::optional<T> get_row(Res &&res)
  {
    int status = PQresultStatus(res.get());

    if (status == PGRES_TUPLES_OK) {

      const auto cols = get_columns_id<T>(res.get());
      int row_count = PQntuples(res.get());
      if (row_count == 0) {
        return {};
      }
      SPHINX_ASSERT(row_count == 1,
                    "Returned none or more than one record ({} records)",
                    row_count);

      return {Db::get_row<T>(res.get(), cols, 0)};
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
        rows.push_back(Db::get_row<T>(res.get(), cols, r));
      }
      return rows;
    }

    logger_->error(PQerrorMessage(conn_.get()));
    throw std::runtime_error(PQerrorMessage(conn_.get()));
  }

  //----------------------------------------------------------------------
  std::vector<const char *> make_pq_args(const ValueList &arguments);

  //----------------------------------------------------------------------

protected:
  Logger logger_;
};

} // namespace Sphinx::Db

//----------------------------------------------------------------------
