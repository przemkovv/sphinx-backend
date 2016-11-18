#pragma once

#include "data.h"

#include <Logger.h>
#include <fmt/format.h>
#include <libpq-fe.h>
#include <nlohmann/json.hpp>

#include <vector>

#include <experimental/propagate_const>
#include <memory>

#include <sphinx_assert.h>

namespace Sphinx::Db {

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

class Db {

public:
  Db(const connection_config &db_config);
  ~Db() = default;

private:
  const connection_config db_config_;

  std::shared_ptr<PGconn> conn_;
  std::shared_ptr<PGconn> connect(const connection_config &db_config);

  template <typename... Args>
  auto exec(const std::string &query, Args... param_args)
  {
    std::vector<std::string> args = make_value_list(param_args...);
    std::vector<const char *> pq_args = make_pq_args(args);

    logger_->info("Executing query: {}", query);

    auto tmp = PQexecParams(conn_.get(), query.data(),
                            static_cast<int>(pq_args.size()), nullptr,
                            pq_args.data(), nullptr, nullptr, 0);
    auto result = std::unique_ptr<PGresult, std::function<void(PGresult *)>>(
        tmp, [=](PGresult *res) { PQclear(res); });

    return std::move(result);
  }

  template <typename T>
  Meta::ColumnsId<T> get_columns_id(PGresult * /* res */);

  template <typename T>
  T get_row(PGresult * /* res */,
            const Meta::ColumnsId<T> & /*cols*/,
            int /*row_id*/);

  template <typename T, typename Res>
  std::vector<T> get_rows(Res &&res);

  std::vector<const char *>
  make_pq_args(const std::vector<std::string> &arguments);

  template <typename... Args>
  std::vector<std::string> make_value_list(Args... args);

  template <typename T>
  std::vector<T> get_all();

  template <typename T>
  T get_field(PGresult *res, int row_id, int col_id);

  template <typename T>
  optional<T> get_field_optional(PGresult *res, int row_id, int col_id);

  template <typename T>
  T to_(const char * /* data */);

public:
  std::vector<User> get_users();
  nlohmann::json get_users_json();
  std::vector<Course> get_courses();
  nlohmann::json get_courses_json();
  // void create_user(const User &user);

private:
  Logger logger_;
};

} // namespace Sphinx::Db
