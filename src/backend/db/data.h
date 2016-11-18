#pragma once

#include <experimental/optional>
#include <string>

namespace Sphinx::Db::Meta {

template <typename T>
struct Columns {
};

template <typename T>
struct ColumnsId {
};

template <typename T>
struct TableName {
};

} // namespace Sphinx::Db::Meta

namespace Sphinx::Db {

using std::experimental::optional;
using std::experimental::nullopt;

template <typename T>
struct Table {
  using Columns = Meta::Columns<T>;
  using ColumnsId = Meta::ColumnsId<T>;
  using TableName = Meta::TableName<T>;
};

//----------------------------------------------------------------------
struct User : public Table<User> {
  int64_t id;
  std::string username;
  std::string email;
};

struct Course : public Table<Course> {
  int64_t id;
  std::string name;
  optional<std::string> description;
};

struct Module : public Table<Module> {
  int64_t id;
  int64_t course_id;
  std::string name;
  optional<std::string> description;
};

} // namespace Sphinx::Db

namespace Sphinx::Db::Meta {

//----------------------------------------------------------------------
template <>
struct TableName<User> {
  static constexpr auto name = "users";
};

template <>
struct Columns<User> {
  using id_t = decltype(User::id);
  using username_t = decltype(User::username);
  using email_t = decltype(User::email);

  static constexpr auto id_n = "id";
  static constexpr auto username_n = "username";
  static constexpr auto email_n = "email";

  static constexpr auto id_o = false;
  static constexpr auto username_o = false;
  static constexpr auto email_o = false;
};

template <>
struct ColumnsId<User> {
  int id;
  int username;
  int email;
};

//----------------------------------------------------------------------
template <>
struct TableName<Course> {
  static constexpr auto name = "courses";
};

template <>
struct Columns<Course> {
  using id_t = decltype(Course::id);
  using name_t = decltype(Course::name);
  using description_t = decltype(Course::description);

  static constexpr auto id_n = "id";
  static constexpr auto name_n = "name";
  static constexpr auto description_n = "description";

  static constexpr auto id_o = false;
  static constexpr auto name_o = false;
  static constexpr auto description_o = true;
};

template <>
struct ColumnsId<Course> {
  int id;
  int name;
  int description;
};

//----------------------------------------------------------------------
template <>
struct TableName<Module> {
  static constexpr auto name = "modules";
};

template <>
struct Columns<Module> {
  using id_t = decltype(Module::id);
  using course_id_t = decltype(Module::course_id);
  using name_t = decltype(Module::name);
  using description_t = decltype(Module::description);

  static constexpr auto id_n = "id";
  static constexpr auto course_id_n = "course_id";
  static constexpr auto name_n = "name";
  static constexpr auto description_n = "description";

  static constexpr auto id_o = false;
  static constexpr auto course_id_o = false;
  static constexpr auto name_o = false;
  static constexpr auto description_o = true;
};

template <>
struct ColumnsId<Module> {
  int id;
  int course_id;
  int name;
  int description;
};

} // namespace Sphinx::Db::Meta
