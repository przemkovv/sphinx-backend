
#include "db.h"

#include "json_serializer.h"

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <utility>

namespace Sphinx::Db {

//----------------------------------------------------------------------
std::shared_ptr<PGconn> Db::connect(const connection_config &db_config)
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
Db::Db(connection_config db_config)
  : db_config_(std::move(db_config)),
    conn_(connect(db_config_)),
    logger_(Sphinx::make_logger("Sphinx::Db", spdlog::level::debug))
{
  if (PQstatus(conn_.get()) != CONNECTION_OK) {
    logger_->error("Cannot connect to database: {}",
                   PQerrorMessage(conn_.get()));
  }
  else {
    logger_->info("Connected to database.");

    for (const auto &table :
         {std::string{User::Table::name}, std::string{Module::Table::name},
          std::string{Course::Table::name}}) {
      auto name = fmt::format("{}", table);
      auto query = fmt::format("SELECT * FROM {}", table);
      prepared_statements.emplace(name, prepare(name, query, 0));
    }
  }
}
//----------------------------------------------------------------------
auto Db::make_result_safe(PGresult *res)
    -> std::unique_ptr<PGresult, std::function<void(PGresult *)>>
{
  return std::unique_ptr<PGresult, std::function<void(PGresult *)>>(
      res, [=](PGresult *res_) { PQclear(res_); });
}

//----------------------------------------------------------------------
template <typename... Args>
auto Db::exec(const std::string &query, Args... param_args)
    -> decltype(make_result_safe(nullptr))
{
  auto args = make_value_list(param_args...);
  auto pq_args = make_pq_args(args);

  logger_->info("Executing query: {}", query);

  auto result =
      PQexecParams(conn_.get(), query.data(), static_cast<int>(pq_args.size()),
                   nullptr, pq_args.data(), nullptr, nullptr, 0);

  return make_result_safe(result);
}

//----------------------------------------------------------------------
template <typename... Args>
auto Db::exec(const prepared_statement &stmt, Args... param_args)
    -> decltype(make_result_safe(nullptr))
{
  auto args = make_value_list(param_args...);
  auto pq_args = make_pq_args(args);

  logger_->info("Executing prepared query: {}", stmt.get_name());

  auto result = PQexecPrepared(conn_.get(), stmt.get_name().c_str(),
                               stmt.get_parameters(), pq_args.data(), nullptr,
                               nullptr, 0);

  return make_result_safe(result);
}
//----------------------------------------------------------------------
prepared_statement
Db::prepare(const std::string &name, const std::string &query, int parameters)
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
std::vector<User> Db::get_users()
{
  return get_all<User>();
}

//----------------------------------------------------------------------
std::vector<Course> Db::get_courses()
{
  return get_all<Course>();
}

//----------------------------------------------------------------------
std::vector<Module> Db::get_modules()
{
  return get_all<Module>();
}

//----------------------------------------------------------------------
template <typename T>
std::vector<T> Db::get_all()
{
  auto prepared_statement = prepared_statements.at(std::string{T::Table::name});
  auto result = exec(prepared_statement);
  // auto result =
  // exec(fmt::format("SELECT * FROM {0}", std::string{T::Table::name}));
  return get_rows<T>(std::move(result));
}

//----------------------------------------------------------------------
template <typename T>
Meta::ColumnsId<T> Db::get_columns_id(PGresult * /* res */)
{
  static_assert(assert_false<T>::value, "Not implemented");
}

//----------------------------------------------------------------------
template <>
Meta::ColumnsId<Module> Db::get_columns_id(PGresult *res)
{
  return {PQfnumber(res, Module::Columns::id_n),
          PQfnumber(res, Module::Columns::course_id_n),
          PQfnumber(res, Module::Columns::name_n),
          PQfnumber(res, Module::Columns::description_n)};
}

//----------------------------------------------------------------------
template <>
Meta::ColumnsId<Course> Db::get_columns_id(PGresult *res)
{
  return {PQfnumber(res, Course::Columns::id_n),
          PQfnumber(res, Course::Columns::name_n),
          PQfnumber(res, Course::Columns::description_n)};
}

//----------------------------------------------------------------------
template <>
Meta::ColumnsId<User> Db::get_columns_id(PGresult *res)
{
  return {PQfnumber(res, User::Columns::id_n),
          PQfnumber(res, User::Columns::username_n),
          PQfnumber(res, User::Columns::email_n)};
}

//----------------------------------------------------------------------
template <typename T>
T Db::get_row(PGresult * /* res */,
              const Meta::ColumnsId<T> & /*cols_id*/,
              int /*row_id*/)
{
  static_assert(assert_false<T>::value, "Not implemented");
}

//----------------------------------------------------------------------
template <>
Course Db::get_row<Course>(PGresult *res,
                           const Meta::ColumnsId<Course> &cols_id,
                           int row_id)
{
  Course course;
  load_field_from_res(course.id, res, row_id, cols_id.id);
  load_field_from_res(course.name, res, row_id, cols_id.name);
  load_field_from_res(course.description, res, row_id, cols_id.description);
  return course;
}

//----------------------------------------------------------------------
template <>
Module Db::get_row<Module>(PGresult *res,
                           const Meta::ColumnsId<Module> &cols_id,
                           int row_id)
{
  Module module;
  load_field_from_res(module.id, res, row_id, cols_id.id);
  load_field_from_res(module.name, res, row_id, cols_id.name);
  load_field_from_res(module.description, res, row_id, cols_id.description);
  load_field_from_res(module.course_id, res, row_id, cols_id.course_id);
  return module;
}
//----------------------------------------------------------------------
template <>
User Db::get_row<User>(PGresult *res,
                       const Meta::ColumnsId<User> &cols_id,
                       int row_id)
{
  User user;
  load_field_from_res(user.id, res, row_id, cols_id.id);
  load_field_from_res(user.username, res, row_id, cols_id.username);
  load_field_from_res(user.email, res, row_id, cols_id.email);
  return user;
}

//----------------------------------------------------------------------
std::vector<const char *>
Db::make_pq_args(const std::vector<optional<std::string>> &arguments)
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
template <typename T>
optional<std::string> to_optional_string(T data)
{
  return {std::to_string(data)};
}

//----------------------------------------------------------------------
template <typename T>
optional<std::string> to_optional_string(const char *data)
{
  return {data};
}

//----------------------------------------------------------------------
template <typename T>
optional<std::string> to_optional_string(const std::string &data)
{
  return {data};
}

//----------------------------------------------------------------------
template <typename T>
optional<std::string> to_optional_string(std::nullptr_t /* null */)
{
  return {};
}

//----------------------------------------------------------------------
template <typename... Args>
std::vector<optional<std::string>> Db::make_value_list(Args... args)
{
  return {to_optional_string(args)...};
}

//----------------------------------------------------------------------
template <typename T, typename Res>
std::vector<T> Db::get_rows(Res &&res)
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
template <typename T>
T Db::get_field(PGresult *res, int row_id, int col_id)
{
  return convert_to<T>(PQgetvalue(res, row_id, col_id));
}

//----------------------------------------------------------------------
template <typename T>
optional<T> Db::get_field_optional(PGresult *res, int row_id, int col_id)
{
  if (PQgetisnull(res, row_id, col_id)) {
    return {};
  }
  return {get_field<T>(res, row_id, col_id)};
}

//----------------------------------------------------------------------
template <typename T>
T Db::convert_to(const char * /* data */)
{
  static_assert(assert_false<T>::value, "Not implemented");
}

//----------------------------------------------------------------------
template <>
int64_t Db::convert_to(const char *data)
{
  return std::stoll(data);
}

//----------------------------------------------------------------------
template <>
std::string Db::convert_to(const char *data)
{
  return {data};
}

} // namespace Sphinx::Db
