#pragma once

#include "model_utils.h"

#include <experimental/optional>
#include <string>


namespace Sphinx::Backend::Model {


//----------------------------------------------------------------------
struct User : public TableDescription<User> {
  int64_t id;
  std::string username;
  std::string email;
};

struct Course : public TableDescription<Course> {
  int64_t id;
  std::string name;
  optional<std::string> description;
};

struct Module : public TableDescription<Module> {
  int64_t id;
  int64_t course_id;
  std::string name;
  optional<std::string> description;
};

} // namespace Sphinx::Backend::Model

namespace Sphinx::Backend::Model::Meta {

//----------------------------------------------------------------------
template <>
struct Table<User> {
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

};

template <>
struct ColumnsId<User> {
  int id;
  int username;
  int email;
};

//----------------------------------------------------------------------
template <>
struct Table<Course> {
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

};

template <>
struct ColumnsId<Course> {
  int id;
  int name;
  int description;
};

//----------------------------------------------------------------------
template <>
struct Table<Module> {
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

};

template <>
struct ColumnsId<Module> {
  int id;
  int course_id;
  int name;
  int description;
};

} // namespace Sphinx::Backend::Model::Meta
