#pragma once

#include "model_meta.h" // for Column, Column<>::name, ForeignKey, optional...
#include <array>
#include <optional> // for optional
#include <stdint.h> // for uint64_t
#include <string>   // for string
#include <tuple>    // for forward_as_tuple, make_tuple
#include <vector>   // for vector

#include <boost/hana/define_struct.hpp>

namespace {
using Sphinx::Db::Column;
using Sphinx::Db::ForeignKey;
using Sphinx::Db::LinkManyFieldType;
using Sphinx::Db::optional_tag;
using Sphinx::Db::primarykey_tag;
using Sphinx::Db::autoincrement_tag;
}

namespace Sphinx::Backend::Model {

struct User;
struct Module;
struct Course;
using Users = Sphinx::Db::Meta::Entities<User>;
using Modules = Sphinx::Db::Meta::Entities<Module>;
using Courses = Sphinx::Db::Meta::Entities<Course>;

//----------------------------------------------------------------------
struct User : Sphinx::Db::Meta::Entity<User> {
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
  BOOST_HANA_DEFINE_STRUCT(
      User,
      (Column<0, User, std::uint64_t, User_::id, primarykey_tag>, id),
      (Column<1, User, std::string, User_::firstname>, firstname),
      (Column<2, User, std::string, User_::lastname>, lastname),
      (Column<3, User, std::string, User_::username>, username),
      (Column<4, User, std::string, User_::student_id, optional_tag>,
       student_id),
      (Column<5, User, std::string, User_::email>, email),
      (Column<6, User, std::string, User_::role>, role));
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
  auto get_many_links() const { return std::forward_as_tuple(); }
  auto get_many_links() { return std::forward_as_tuple(); }
};

//----------------------------------------------------------------------
struct Course : Sphinx::Db::Meta::Entity<Course> {
public:
  struct Course_ {
    static constexpr char id[] = "id";
    static constexpr char title[] = "title";
    static constexpr char description[] = "description";
    static constexpr char owner_id[] = "owner_id";

    static constexpr char modules[] = "modules";
  };

public:
  BOOST_HANA_DEFINE_STRUCT(
      Course,
      (Column<0, Course, std::uint64_t, Course_::id, primarykey_tag>, id),
      (Column<1, Course, std::string, Course_::title>, title),
      (Column<2, Course, std::string, Course_::description, optional_tag>,
       description),
      (ForeignKey<3, User, decltype(User::id), Course, Course_::owner_id>,
       owner_id));
  constexpr static const auto N = 4;

  auto get_columns() const
  {
    return std::forward_as_tuple(id, title, description, owner_id);
  }
  auto get_columns()
  {
    return std::forward_as_tuple(id, title, description, owner_id);
  }

  LinkManyFieldType<Modules> modules;

  auto get_many_links() const { return std::forward_as_tuple(modules); }
  auto get_many_links() { return std::forward_as_tuple(modules); }
};

//----------------------------------------------------------------------
struct Module : Sphinx::Db::Meta::Entity<Module> {
private:
  struct Module_ {
    static constexpr char id[] = "id";
    static constexpr char course_id[] = "course_id";
    static constexpr char title[] = "title";
    static constexpr char description[] = "description";
  };

public:
  BOOST_HANA_DEFINE_STRUCT(
      Module,
      (Column<0, Module, std::uint64_t, Module_::id, primarykey_tag>, id),
      (ForeignKey<1, Course, decltype(Course::id), Module, Module_::course_id>,
       course_id),
      (Column<2, Module, std::string, Module_::title>, title),
      (Column<3, Module, std::string, Module_::description, optional_tag>,
       description));

  constexpr static const auto N = 4;

  auto get_columns() const
  {
    return std::forward_as_tuple(id, course_id, title, description);
  }
  auto get_columns()
  {
    return std::forward_as_tuple(id, course_id, title, description);
  }
  auto get_many_links() const { return std::forward_as_tuple(); }
  auto get_many_links() { return std::forward_as_tuple(); }
};

} // namespace Sphinx::Backend::Model

namespace Sphinx::Db::Meta {

//----------------------------------------------------------------------
template <>
constexpr auto EntityName<Backend::Model::User> = "users";

//----------------------------------------------------------------------
template <>
constexpr auto EntityName<Backend::Model::Course> = "courses";

//----------------------------------------------------------------------
template <>
constexpr auto EntityName<Backend::Model::Module> = "modules";

//----------------------------------------------------------------------

} // namespace Sphinx::Backend::Model::Meta
