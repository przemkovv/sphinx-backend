#pragma once

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
// using namespace Sphinx::Backend::Model;

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
  template <typename... Args>
  auto exec(const std::string &query, Args... param_args)
      -> decltype(make_result_safe(nullptr));

  template <typename... Args>
  auto exec(const prepared_statement &stmt, Args... param_args)
      -> decltype(make_result_safe(nullptr));

  prepared_statement
  prepare(const std::string &name, const std::string &query, int parameters);

public:
  template <typename T>
  std::vector<T> get_all();

  template <typename T, typename Res>
  std::vector<T> get_rows(Res &&res);

  std::vector<const char *>
  make_pq_args(const std::vector<optional<std::string>> &arguments);

  template <typename... Args>
  auto make_value_list(Args... args) -> std::vector<optional<std::string>>;

  template <typename T>
  optional<std::string> to_optional_string(T data);

  // void create_user(const User &user);

private:
  Logger logger_;
};

} // namespace Sphinx::Db

//----------------------------------------------------------------------
namespace Sphinx::Db {

template <typename T>
T get_row(PGresult * /* res */,
          const Sphinx::Db::Meta::ColumnsId<T> & /*cols*/,
          int /*row_id*/);

template <typename T>
Sphinx::Db::Meta::ColumnsId<T> get_columns_id(PGresult * /* res */);

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
} // namespace Sphinx::Db

//----------------------------------------------------------------------
namespace Sphinx::Backend::Db {

class BackendDb : public Sphinx::Db::DbConnection {
public:
  BackendDb(Sphinx::Db::connection_config config)
    : DbConnection(std::move(config))
  {
    for (const auto &table : {std::string{Model::User::Table::name},
                              std::string{Model::Module::Table::name},
                              std::string{Model::Course::Table::name}}) {
      auto name = fmt::format("{}", table);
      auto query = fmt::format("SELECT * FROM {}", table);
      prepared_statements.emplace(name, prepare(name, query, 0));
    }
  }

private:
  std::map<std::string, Sphinx::Db::prepared_statement> prepared_statements;

public:
  std::vector<Model::User> get_users();
  std::vector<Model::Course> get_courses();
  std::vector<Model::Module> get_modules();
};

} // namespace Sphinx::Backend::Db
