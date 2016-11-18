#pragma once

#include <experimental/optional>
#include <string>

namespace Sphinx::Db::Meta {

template <typename T>
struct ColumnsName {
};

template <typename T>
struct ColumnsId {
};

template <typename T>
struct TableName {
};

template <typename T>
struct Nullable {
};
} // namespace Sphinx::Db::Meta

namespace Sphinx::Db {

using std::experimental::optional;
using std::experimental::nullopt;;

template <typename T>
struct Table {
  using ColumnsName = Meta::ColumnsName<T>;
  using ColumnsId = Meta::ColumnsId<T>;
  using TableName = Meta::TableName<T>;
  using Nullable = Meta::Nullable<T>;
};

struct User : public Table<User> {
  int64_t id;
  std::string username;
  std::string email;

};

struct Course : public Table<Course>{
  int64_t id;
  std::string name;
  optional<std::string> description;
};

struct Module : public Table<Module>{
  int64_t id;
  int64_t course_id;
  std::string name;
  optional<std::string> description;
};

} // namespace Sphinx::Db

namespace Sphinx::Db::Meta {

template <>
struct TableName<User> {
  static constexpr auto name = "users";
};

template <>
struct ColumnsName<User> {
  static constexpr auto id = "id";
  static constexpr auto username = "username";
  static constexpr auto email = "email";
};

template <>
struct Nullable<User> {
  static constexpr auto id = false;
  static constexpr auto username = false;
  static constexpr auto email = false;
};

template <>
struct ColumnsId<User> {
  int id;
  int username;
  int email;
};

template <>
struct TableName<Course> {
  static constexpr auto name = "courses";
};

template <>
struct ColumnsName<Course> {
  static constexpr auto id = "id";
  static constexpr auto name = "name";
  static constexpr auto description = "description";
};

template <>
struct Nullable<Course> {
  static constexpr auto id = false;
  static constexpr auto name = false;
  static constexpr auto description = true;
};

template <>
struct ColumnsId<Course> {
  int id;
  int name;
  int description;
};

template <>
struct TableName<Module> {
  static constexpr auto name = "modules";
};

template <>
struct ColumnsName<Module> {
  static constexpr auto id = "id";
  static constexpr auto course_id = "course_id";
  static constexpr auto name = "name";
  static constexpr auto description = "description";
};

template <>
struct Nullable<Module> {
  static constexpr auto id = false;
  static constexpr auto course_id = false;
  static constexpr auto name = false;
  static constexpr auto description = true;
};

template <>
struct ColumnsId<Module> {
  int id;
  int course_id;
  int name;
  int description;
};

} // namespace Sphinx::Db::Meta
