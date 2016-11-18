
#include "db.h"

#include "json_serializer.h"

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <utility>

namespace Sphinx::Db {

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

Db::Db(const connection_config &db_config)
  : db_config_(db_config),
    conn_(connect(db_config_)),
    logger_(Sphinx::make_logger("Sphinx::Db", spdlog::level::debug))
{
  if (PQstatus(conn_.get()) != CONNECTION_OK) {
    logger_->error("Cannot connect to database: {}",
                   PQerrorMessage(conn_.get()));
  }
  else {
    logger_->info("Connected to database.");
  }
}

nlohmann::json Db::get_courses_json()
{
  auto courses = get_courses();
  return to_json(courses);
}
nlohmann::json Db::get_users_json()
{
  auto users = get_users();
  return to_json(users);
}

template <>
int64_t Db::to_(const char *data)
{
  return std::stoll(data);
}
template <>
std::string Db::to_(const char *data)
{
  return {data};
}

template <typename T>
std::vector<T> Db::get_all()
{
  auto result =
      exec(fmt::format("SELECT * FROM {0}", std::string{T::TableName::name}));
  return get_rows<T>(std::move(result));
}

std::vector<User> Db::get_users() { return get_all<User>(); }
std::vector<Course> Db::get_courses() { return get_all<Course>(); }

template <>
Meta::ColumnsId<Course> Db::get_columns_id(PGresult *res)
{
  return {PQfnumber(res, Course::ColumnsName::id),
          PQfnumber(res, Course::ColumnsName::name),
          PQfnumber(res, Course::ColumnsName::description)};
}

template <>
Course Db::get_row<Course>(PGresult *res,
                           const Meta::ColumnsId<Course> &cols,
                           int row_id)
{
  Course course;
  course.id = get_field<int64_t>(res, row_id, cols.id);
  course.name = get_field<std::string>(res, row_id, cols.name);
  course.description =
      get_field_optional<std::string>(res, row_id, cols.description);

  return course;
}

template <>
Meta::ColumnsId<User> Db::get_columns_id(PGresult *res)
{
  return {PQfnumber(res, User::ColumnsName::id),
          PQfnumber(res, User::ColumnsName::username),
          PQfnumber(res, User::ColumnsName::email)};
}

template <>
User Db::get_row<User>(PGresult *res,
                       const Meta::ColumnsId<User> &cols,
                       int row_id)
{
  User user;
  user.id = get_field<int64_t>(res, row_id, cols.id);
  user.username = get_field<std::string>(res, row_id, cols.username);
  user.email = get_field<std::string>(res, row_id, cols.email);

  return user;
}

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

template <typename T>
Meta::ColumnsId<T> Db::get_columns_id(PGresult * /* res */)
{
  static_assert(assert_false<T>::value, "Not implemented");
}

template <typename T>
T Db::get_row(PGresult * /* res */,
              const Meta::ColumnsId<T> & /*cols*/,
              int /*row_id*/)
{
  static_assert(assert_false<T>::value, "Not implemented");
}

std::vector<const char *>
Db::make_pq_args(const std::vector<std::string> &arguments)
{
  std::vector<const char *> pq_args;

  for (auto &val : arguments) {
    pq_args.push_back(val.data());
  }

  return pq_args;
}

template <typename... Args>
std::vector<std::string> Db::make_value_list(Args... args)
{
  std::vector<std::string> list{std::to_string(args)...};
  return list;
}

template <typename T>
T Db::get_field(PGresult *res, int row_id, int col_id)
{
  return to_<T>(PQgetvalue(res, row_id, col_id));
}

template <typename T>
optional<T> Db::get_field_optional(PGresult *res, int row_id, int col_id)
{
  if (PQgetisnull(res, row_id, col_id)) {
    return {};
  }
  else {
    return {get_field<T>(res, row_id, col_id)};
  }
}

template <typename T>
T Db::to_(const char * /* data */)
{
  static_assert(assert_false<T>::value, "Not implemented");
}

} // namespace Sphinx::Db
