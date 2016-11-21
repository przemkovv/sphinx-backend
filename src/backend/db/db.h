#pragma once

#include "data.h"

#include "Logger.h"
#include "utils.h"
#include <fmt/format.h>
#include <libpq-fe.h>
#include <nlohmann/json.hpp>

#include <vector>

#include <experimental/propagate_const>
#include <memory>

#include "sphinx_assert.h"

namespace Sphinx::Db {

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
class Db {

public:
  Db(connection_config db_config);
  ~Db() = default;

private:
  /* database connection and statement execution */
  const connection_config db_config_;
  std::map<std::string, prepared_statement> prepared_statements;

  std::shared_ptr<PGconn> conn_;
  std::shared_ptr<PGconn> connect(const connection_config &db_config);

  auto make_result_safe(PGresult *res)
      -> std::unique_ptr<PGresult, std::function<void(PGresult *)>>;

  template <typename... Args>
  auto exec(const std::string &query, Args... param_args)
      -> decltype(make_result_safe(nullptr));

  template <typename... Args>
  auto exec(const prepared_statement &stmt, Args... param_args)
      -> decltype(make_result_safe(nullptr));

  prepared_statement
  prepare(const std::string &name, const std::string &query, int parameters);

private:
  template <typename T>
  std::vector<T> get_all();

  template <typename T, typename Res>
  std::vector<T> get_rows(Res &&res);

  template <typename T>
  T get_row(PGresult * /* res */,
            const Meta::ColumnsId<T> & /*cols*/,
            int /*row_id*/);

  template <typename T>
  Meta::ColumnsId<T> get_columns_id(PGresult * /* res */);

  std::vector<const char *>
  make_pq_args(const std::vector<optional<std::string>> &arguments);

  template <typename... Args>
  auto make_value_list(Args... args) -> std::vector<optional<std::string>>;

  template <typename T>
  T get_field(PGresult *res, int row_id, int col_id);

  template <typename T>
  optional<T> get_field_optional(PGresult *res, int row_id, int col_id);

  template <typename T>
  auto get_field_c(PGresult *res, int row_id, int col_id)
  {
    if
      constexpr(Utils::is_optional<T>::value)
      {
        return get_field_optional<typename Utils::remove_optional<T>::type>(
            res, row_id, col_id);
      }
    else {
      return get_field<T>(res, row_id, col_id);
    }
  }

  template <typename T>
  void load_field_from_res(T &field, PGresult *res, int row_id, int col_id)
  {
    field = get_field_c<T>(res, row_id, col_id);
  }

  template <typename T>
  T convert_to(const char * /* data */);

  template <typename T>
  optional<std::string> to_optional_string(T data);

public:
  std::vector<User> get_users();
  std::vector<Course> get_courses();
  std::vector<Module> get_modules();
  // void create_user(const User &user);

private:
  Logger logger_;
};

} // namespace Sphinx::Db
