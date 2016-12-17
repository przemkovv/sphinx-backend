#pragma once

#include "db/model_meta.h" // for Column, Column<>::name, ForeignKey, optional...
#include "db/model_relations.h"
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
public:
  BOOST_HANA_DEFINE_STRUCT(User,
                           (Column<0, User, std::uint64_t, primarykey_tag>, id),
                           (Column<1, User, std::string>, firstname),
                           (Column<2, User, std::string>, lastname),
                           (Column<3, User, std::string>, username),
                           (Column<4, User, std::string, optional_tag>,
                            student_id),
                           (Column<5, User, std::string>, email),
                           (Column<6, User, std::string>, role));
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
  BOOST_HANA_DEFINE_STRUCT(
      Course,
      (Column<0, Course, std::uint64_t, primarykey_tag>, id),
      (Column<1, Course, std::string>, title),
      (Column<2, Course, std::string, optional_tag>, description),
      (ForeignKey<3, User, decltype(User::id), Course>, owner_id));
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
public:
  BOOST_HANA_DEFINE_STRUCT(
      Module,
      (Column<0, Module, std::uint64_t, primarykey_tag>, id),
      (ForeignKey<1, Course, decltype(Course::id), Module>, course_id),
      (Column<2, Module, std::string>, title),
      (Column<3, Module, std::string, optional_tag>, description));

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
