#pragma once

#include "model_meta.h" // for ColumnsId (ptr only), Insert (ptr only), Column
#include <tuple>

using Sphinx::Db::Column;

namespace Sphinx::Backend::Model {

using Sphinx::Db::just_tag;
using Sphinx::Db::optional_tag;
using Sphinx::Db::autoincrement_tag;

//----------------------------------------------------------------------
struct User {
private:
  struct User_ {
    static constexpr char id[] = "id";
    static constexpr char username[] = "username";
    static constexpr char email[] = "email";
  };

public:
  Column<User, std::int64_t, User_::id> id;
  Column<User, std::string, User_::username> username;
  Column<User, std::string, User_::email> email;
};

//----------------------------------------------------------------------
struct Course {
private:
  struct Course_ {
    static constexpr char id[] = "id";
    static constexpr char name[] = "name";
    static constexpr char description[] = "description";
  };

public:
  Column<Course, std::int64_t, Course_::id> id;
  Column<Course, std::string, Course_::name> name;
  Column<Course, std::string, Course_::description, optional_tag> description;
};

//----------------------------------------------------------------------
struct Module {
private:
  struct Module_ {
    static constexpr char id[] = "id";
    static constexpr char course_id[] = "course_id";
    static constexpr char name[] = "name";
    static constexpr char description[] = "description";
  };

public:
  Column<Module, std::int64_t, Module_::id> id;
  Column<Module, std::int64_t, Module_::course_id> course_id;
  Column<Module, std::string, Module_::name> name;
  Column<Module, std::string, Module_::description, optional_tag> description;
};

} // namespace Sphinx::Backend::Model

namespace Sphinx::Db::Meta {

//----------------------------------------------------------------------

template <>
constexpr auto TableName<Backend::Model::User> = "users";

template <>
struct Insert<Backend::Model::User> {
  using User = Backend::Model::User;

  static constexpr auto columns = {decltype(User::username)::name,
                                   decltype(User::email)::name};
  static auto values(const User &data)
  {
    return std::make_tuple(data.username.value, data.email.value);
  }
  using id_column = decltype(User::id);
};

//----------------------------------------------------------------------
template <>
constexpr auto TableName<Backend::Model::Course> = "courses";

template <>
struct Insert<Backend::Model::Course> {
  using Course = Backend::Model::Course;

  static constexpr auto columns = {decltype(Course::name)::name,
                                   decltype(Course::description)::name};
  static auto values(const Course &data)
  {
    return std::make_tuple(data.name.value, data.description.value);
  }
  using id_column = decltype(Course::id);
};

//----------------------------------------------------------------------
template <>
constexpr auto TableName<Backend::Model::Module> = "modules";

template <>
struct Insert<Backend::Model::Module> {
  using Module = Backend::Model::Module;

  static constexpr auto columns = {decltype(Module::course_id)::name,
                                   decltype(Module::name)::name,
                                   decltype(Module::description)::name};
  static auto values(const Module &data)
  {
    return std::make_tuple(data.course_id.value, data.name.value,
                           data.description.value);
  }
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
