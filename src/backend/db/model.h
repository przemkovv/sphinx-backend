#pragma once

#include "model_meta.h" // for ColumnsId (ptr only), Insert (ptr only), Column
#include <tuple>

using Sphinx::Db::Column;
using Sphinx::Db::ForeignKey;

namespace Sphinx::Backend::Model {

using Sphinx::Db::optional_tag;
using Sphinx::Db::autoincrement_tag;

//----------------------------------------------------------------------
struct User : Sphinx::Db::Meta::Table<User> {
private:
  struct User_ {
    static constexpr char id[] = "id";
    static constexpr char username[] = "username";
    static constexpr char email[] = "email";
    static constexpr char firstname[] = "firstname";
    static constexpr char lastname[] = "lastname";
    static constexpr char student_id[] = "student_id";
    static constexpr char role[] = "role";
  };

public:
  Column<0, User, std::uint64_t, User_::id, optional_tag> id;
  Column<1, User, std::string, User_::firstname> firstname;
  Column<2, User, std::string, User_::lastname> lastname;
  Column<3, User, std::string, User_::username> username;
  Column<4, User, std::string, User_::student_id, optional_tag> student_id;
  Column<5, User, std::string, User_::email> email;
  Column<6, User, std::string, User_::role> role;
  constexpr static const auto N = 7;

  auto get_columns() const
  {
    return std::forward_as_tuple(id, firstname, lastname, username, student_id,
                                 email, role);
  }
  auto get_columns()
  {
    return std::forward_as_tuple(id, firstname, lastname, username, student_id,
                                 email, role);
  }
};

//----------------------------------------------------------------------
struct Course {
private:
  struct Course_ {
    static constexpr char id[] = "id";
    static constexpr char title[] = "title";
    static constexpr char description[] = "description";
    static constexpr char owner_id[] = "owner_id";
  };

public:
  Column<0, Course, std::uint64_t, Course_::id, optional_tag> id;
  Column<1, Course, std::string, Course_::title> title;
  Column<2, Course, std::string, Course_::description, optional_tag>
      description;
  ForeignKey<3, User, decltype(User::id), Course, Course_::owner_id> owner_id;
  constexpr static const auto N = 4;

  auto get_columns() const
  {
    return std::forward_as_tuple(id, title, description, owner_id);
  }
  auto get_columns()
  {
    return std::forward_as_tuple(id, title, description, owner_id);
  }
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
  Column<0, Module, std::uint64_t, Module_::id, optional_tag> id;
  ForeignKey<1, Course, decltype(Course::id), Module, Module_::course_id>
      course_id;
  Column<2, Module, std::string, Module_::name> name;
  Column<3, Module, std::string, Module_::description, optional_tag>
      description;

  constexpr static const auto N = 4;

  auto get_columns() const
  {
    return std::forward_as_tuple(id, course_id, name, description);
  }
  auto get_columns()
  {
    return std::forward_as_tuple(id, course_id, name, description);
  }
};

} // namespace Sphinx::Backend::Model

namespace Sphinx::Db::Meta {

//----------------------------------------------------------------------

template <>
constexpr auto TableName<Backend::Model::User> = "users";

template <>
struct Insert<Backend::Model::User> {
  using User = Backend::Model::User;

  static constexpr auto columns = {
      decltype(User::firstname)::name, decltype(User::lastname)::name,
      decltype(User::username)::name,  decltype(User::student_id)::name,
      decltype(User::email)::name,     decltype(User::role)::name};
  static auto values(const User &data)
  {
    return std::make_tuple(data.firstname.value, data.lastname.value,
                           data.username.value, data.student_id.value,
                           data.email.value, data.role.value);
  }
  using id_column = decltype(User::id);
};

//----------------------------------------------------------------------
template <>
constexpr auto TableName<Backend::Model::Course> = "courses";

template <>
struct Insert<Backend::Model::Course> {
  using Course = Backend::Model::Course;

  static constexpr auto columns = {decltype(Course::title)::name,
                                   decltype(Course::description)::name,
                                   decltype(Course::owner_id)::name};
  static auto values(const Course &data)
  {
    return std::make_tuple(data.title.value, data.description.value,
                           data.owner_id.value);
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

// template <>
// struct ColumnsId<Backend::Model::User> {
// int id;
// int firstname;
// int lastname;
// int student_id;
// int username;
// int email;
// int role;
// };
// template <>
// struct ColumnsId<Backend::Model::Course> {
// int id;
// int name;
// int description;
// };
// template <>
// struct ColumnsId<Backend::Model::Module> {
// int id;
// int course_id;
// int name;
// int description;
// };

} // namespace Sphinx::Backend::Model::Meta
