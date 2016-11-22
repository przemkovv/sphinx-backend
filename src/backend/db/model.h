#pragma once

#include "model_utils.h"         // for Columns (ptr only), ColumnsId (ptr ...
#include <experimental/optional> // for optional
#include <stdint.h>              // for int64_t
#include <string>                // for string

namespace Sphinx::Backend::Model {

//----------------------------------------------------------------------
struct User : public Db::TableDescription<User> {
  int64_t id;
  std::string username;
  std::string email;
};

struct Course : public Db::TableDescription<Course> {
  int64_t id;
  std::string name;
  Db::optional<std::string> description;
};

struct Module : public Db::TableDescription<Module> {
  int64_t id;
  int64_t course_id;
  std::string name;
  Db::optional<std::string> description;
};

} // namespace Sphinx::Backend::Model

namespace Sphinx::Db::Meta {

//----------------------------------------------------------------------
template <>
struct Table<Backend::Model::User> {
  static constexpr auto name = "users";
};

template <>
struct Columns<Backend::Model::User> {
  using User = Backend::Model::User;
  using id_t = decltype(User::id);
  using username_t = decltype(User::username);
  using email_t = decltype(User::email);

  static constexpr auto id_n = "id";
  static constexpr auto username_n = "username";
  static constexpr auto email_n = "email";

  static constexpr auto insert_columns = {username_n, email_n};
  static constexpr auto id_column = id_n;
  using id_column_t = id_t;
};

template <>
struct ColumnsId<Backend::Model::User> {
  int id;
  int username;
  int email;
};

//----------------------------------------------------------------------
template <>
struct Table<Backend::Model::Course> {
  static constexpr auto name = "courses";
};

template <>
struct Columns<Backend::Model::Course> {
  using Course = Backend::Model::Course;
  using id_t = decltype(Course::id);
  using name_t = decltype(Course::name);
  using description_t = decltype(Course::description);

  static constexpr auto id_n = "id";
  static constexpr auto name_n = "name";
  static constexpr auto description_n = "description";

  static constexpr auto insert_columns = {name_n, description_n};
  static constexpr auto id_column = id_n;
  using id_column_t = id_t;
};

template <>
struct ColumnsId<Backend::Model::Course> {
  int id;
  int name;
  int description;
};

//----------------------------------------------------------------------
template <>
struct Table<Backend::Model::Module> {
  static constexpr auto name = "modules";
};

template <>
struct Columns<Backend::Model::Module> {
  using Module = Backend::Model::Module;
  using id_t = decltype(Module::id);
  using course_id_t = decltype(Module::course_id);
  using name_t = decltype(Module::name);
  using description_t = decltype(Module::description);

  static constexpr auto id_n = "id";
  static constexpr auto course_id_n = "course_id";
  static constexpr auto name_n = "name";
  static constexpr auto description_n = "description";

  static constexpr auto insert_columns = {course_id_n, name_n, description_n};
  static constexpr auto id_column = id_n;
  using id_column_t = id_t;
};

template <>
struct ColumnsId<Backend::Model::Module> {
  int id;
  int course_id;
  int name;
  int description;
};

} // namespace Sphinx::Backend::Model::Meta
