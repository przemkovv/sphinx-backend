#pragma once

#include "model_meta.h"          // for Columns (ptr only), ColumnsId (ptr ...
#include <experimental/optional> // for optional
#include <stdint.h>              // for int64_t
#include <string>                // for string

using Sphinx::Db::Meta::Column;

namespace Sphinx::Backend::Model {

struct Course_ {
  static constexpr char id[] = "id";
  static constexpr char name[] = "name";
  static constexpr char description[] = "description";
};

//----------------------------------------------------------------------
struct Module_ {
  static constexpr char id[] = "id";
  static constexpr char course_id[] = "course_id";
  static constexpr char name[] = "name";
  static constexpr char description[] = "description";
};

struct User_ {
  static constexpr char id[] = "id";
  static constexpr char username[] = "username";
  static constexpr char email[] = "email";
};

//----------------------------------------------------------------------
struct User : public Db::TableDescription<User> {
  Column<User, int64_t, User_::id> id;
  Column<User, std::string, User_::username> username;
  Column<User, std::string, User_::email> email;
};

struct Course : public Db::TableDescription<Course> {
  Column<Course, int64_t, Course_::id> id;
  Column<Course, std::string, Course_::name> name;
  Column<Course, Db::optional<std::string>, Course_::description> description;
};

struct Module : public Db::TableDescription<Module> {
  Column<Module, int64_t, Module_::id> id;
  Column<Module, int64_t, Module_::course_id> course_id;
  Column<Module, std::string, Module_::name> name;
  Column<Module, Db::optional<std::string>, Module_::description> description;
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

  static constexpr auto insert_columns = {decltype(User::username)::name,
                                          decltype(User::email)::name};
  using id_column = decltype(User::id);
};

//----------------------------------------------------------------------
template <>
struct Table<Backend::Model::Course> {
  static constexpr auto name = "courses";
};

template <>
struct Columns<Backend::Model::Course> {
  using Course = Backend::Model::Course;

  static constexpr auto insert_columns = {decltype(Course::name)::name,
                                          decltype(Course::description)::name};
  using id_column = decltype(Course::id);
};

//----------------------------------------------------------------------
template <>
struct Table<Backend::Model::Module> {
  static constexpr auto name = "modules";
};

template <>
struct Columns<Backend::Model::Module> {
  using Module = Backend::Model::Module;

  static constexpr auto insert_columns = {decltype(Module::course_id)::name,
                                          decltype(Module::name)::name,
                                          decltype(Module::description)::name};
  using id_column = decltype(Module::id);
};

//----------------------------------------------------------------------
template <>
struct ColumnsId<Backend::Model::User> {
  int id;
  int username;
  int email;
};
template <>
struct ColumnsId<Backend::Model::Course> {
  int id;
  int name;
  int description;
};
template <>
struct ColumnsId<Backend::Model::Module> {
  int id;
  int course_id;
  int name;
  int description;
};

} // namespace Sphinx::Backend::Model::Meta
