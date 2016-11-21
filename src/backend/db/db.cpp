
#include "db.h"

#include "json_serializer.h"

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <utility>

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
template <typename... Args>
auto DbConnection::exec(const std::string &query, Args... param_args)
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
auto DbConnection::exec(const prepared_statement &stmt, Args... param_args)
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
template <typename T>
std::vector<T> DbConnection::get_all()
{
  auto result =
      exec(fmt::format("SELECT * FROM {0}", std::string{T::Table::name}));
  return get_rows<T>(std::move(result));
}

//----------------------------------------------------------------------
template <typename T>
Meta::ColumnsId<T> get_columns_id(PGresult * /* res */)
{
  static_assert(assert_false<T>::value, "Not implemented");
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
template <typename T>
T get_row(PGresult * /* res */,
          const Meta::ColumnsId<T> & /*cols_id*/,
          int /*row_id*/)
{
  static_assert(assert_false<T>::value, "Not implemented");
}

//----------------------------------------------------------------------
std::vector<const char *>
DbConnection::make_pq_args(const std::vector<optional<std::string>> &arguments)
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
std::vector<optional<std::string>> DbConnection::make_value_list(Args... args)
{
  return {to_optional_string(args)...};
}

//----------------------------------------------------------------------
template <typename T, typename Res>
std::vector<T> DbConnection::get_rows(Res &&res)
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
T get_field(PGresult *res, int row_id, int col_id)
{
  return convert_to<T>(PQgetvalue(res, row_id, col_id));
}

//----------------------------------------------------------------------
template <typename T>
optional<T> get_field_optional(PGresult *res, int row_id, int col_id)
{
  if (PQgetisnull(res, row_id, col_id)) {
    return {};
  }
  return {get_field<T>(res, row_id, col_id)};
}

//----------------------------------------------------------------------
template <typename T>
T convert_to(const char * /* data */)
{
  static_assert(assert_false<T>::value, "Not implemented");
}

//----------------------------------------------------------------------
template <>
int64_t convert_to(const char *data)
{
  return std::stoll(data);
}

//----------------------------------------------------------------------
template <>
std::string convert_to(const char *data)
{
  return {data};
}

} // namespace Sphinx::Db

namespace Sphinx::Db {

template <>
Db::Meta::ColumnsId<Backend::Model::Module> get_columns_id(PGresult *res)
{
  return {PQfnumber(res, Backend::Model::Module::Columns::id_n),
          PQfnumber(res, Backend::Model::Module::Columns::course_id_n),
          PQfnumber(res, Backend::Model::Module::Columns::name_n),
          PQfnumber(res, Backend::Model::Module::Columns::description_n)};
}

//----------------------------------------------------------------------
template <>
Db::Meta::ColumnsId<Backend::Model::Course> get_columns_id(PGresult *res)
{
  return {PQfnumber(res, Backend::Model::Course::Columns::id_n),
          PQfnumber(res, Backend::Model::Course::Columns::name_n),
          PQfnumber(res, Backend::Model::Course::Columns::description_n)};
}

//----------------------------------------------------------------------
template <>
Db::Meta::ColumnsId<Backend::Model::User> get_columns_id(PGresult *res)
{
  return {PQfnumber(res, Backend::Model::User::Columns::id_n),
          PQfnumber(res, Backend::Model::User::Columns::username_n),
          PQfnumber(res, Backend::Model::User::Columns::email_n)};
}

//----------------------------------------------------------------------
template <>
Backend::Model::Course get_row<Backend::Model::Course>(
    PGresult *res,
    const Db::Meta::ColumnsId<Backend::Model::Course> &cols_id,
    int row_id)
{
  Backend::Model::Course course;
  load_field_from_res(course.id, res, row_id, cols_id.id);
  load_field_from_res(course.name, res, row_id, cols_id.name);
  load_field_from_res(course.description, res, row_id, cols_id.description);
  return course;
}

//----------------------------------------------------------------------
template <>
Backend::Model::Module get_row<Backend::Model::Module>(
    PGresult *res,
    const Db::Meta::ColumnsId<Backend::Model::Module> &cols_id,
    int row_id)
{
  Backend::Model::Module module;
  load_field_from_res(module.id, res, row_id, cols_id.id);
  load_field_from_res(module.name, res, row_id, cols_id.name);
  load_field_from_res(module.description, res, row_id, cols_id.description);
  load_field_from_res(module.course_id, res, row_id, cols_id.course_id);
  return module;
}
//----------------------------------------------------------------------
template <>
Backend::Model::User get_row<Backend::Model::User>(
    PGresult *res,
    const Db::Meta::ColumnsId<Backend::Model::User> &cols_id,
    int row_id)
{
  Backend::Model::User user;
  load_field_from_res(user.id, res, row_id, cols_id.id);
  load_field_from_res(user.username, res, row_id, cols_id.username);
  load_field_from_res(user.email, res, row_id, cols_id.email);
  return user;
}

} // namespace Sphinx::Db

//----------------------------------------------------------------------
namespace Sphinx::Backend::Db {

std::vector<Model::User> BackendDb::get_users()
{
  return get_all<Model::User>();
}

//----------------------------------------------------------------------
std::vector<Model::Course> BackendDb::get_courses()
{
  return get_all<Model::Course>();
}

//----------------------------------------------------------------------
std::vector<Model::Module> BackendDb::get_modules()
{
  return get_all<Model::Module>();
}
} // namespace Sphinx::Backend::Db
