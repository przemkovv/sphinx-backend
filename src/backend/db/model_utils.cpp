#include "model_utils.h"

namespace Sphinx::Db {

template <>
Meta::ColumnsId<Backend::Model::Module> get_columns_id(PGresult *res)
{
  using E = Backend::Model::Module;
  return {PQfnumber(res, decltype(E::id)::name),
          PQfnumber(res, decltype(E::course_id)::name),
          PQfnumber(res, decltype(E::name)::name),
          PQfnumber(res, decltype(E::description)::name)};
}

//----------------------------------------------------------------------
template <>
Meta::ColumnsId<Backend::Model::Course> get_columns_id(PGresult *res)
{
  using E = Backend::Model::Course;
  return {PQfnumber(res, decltype(E::id)::name),
          PQfnumber(res, decltype(E::name)::name),
          PQfnumber(res, decltype(E::description)::name)};
}

//----------------------------------------------------------------------
template <>
Meta::ColumnsId<Backend::Model::User> get_columns_id(PGresult *res)
{
  using E = Backend::Model::User;
  return {PQfnumber(res, decltype(E::id)::name),
          PQfnumber(res, decltype(E::firstname)::name),
          PQfnumber(res, decltype(E::lastname)::name),
          PQfnumber(res, decltype(E::student_id)::name),
          PQfnumber(res, decltype(E::username)::name),
          PQfnumber(res, decltype(E::email)::name),
          PQfnumber(res, decltype(E::role)::name)};
}

//----------------------------------------------------------------------
template <>
Backend::Model::Course get_row<Backend::Model::Course>(
    PGresult *res,
    const Meta::ColumnsId<Backend::Model::Course> &cols_id,
    int row_id)
{
  Backend::Model::Course course;
  load_field_from_res(course.id, res, row_id, cols_id[course.id.n]);
  load_field_from_res(course.name, res, row_id, cols_id[course.name.n]);
  load_field_from_res(course.description, res, row_id,
                      cols_id[course.description.n]);
  return course;
}

//----------------------------------------------------------------------
template <>
Backend::Model::Module get_row<Backend::Model::Module>(
    PGresult *res,
    const Meta::ColumnsId<Backend::Model::Module> &cols_id,
    int row_id)
{
  Backend::Model::Module module;
  load_field_from_res(module.id, res, row_id, cols_id[module.id.n]);
  load_field_from_res(module.name, res, row_id, cols_id[module.name.n]);
  load_field_from_res(module.description, res, row_id,
                      cols_id[module.description.n]);
  load_field_from_res(module.course_id, res, row_id,
                      cols_id[module.course_id.n]);
  return module;
}
//----------------------------------------------------------------------
template <>
Backend::Model::User get_row<Backend::Model::User>(
    PGresult *res,
    const Meta::ColumnsId<Backend::Model::User> &cols_id,
    int row_id)
{
  Backend::Model::User user;
  load_field_from_res(user.id, res, row_id, cols_id[user.id.n]);
  load_field_from_res(user.firstname, res, row_id, cols_id[user.firstname.n]);
  load_field_from_res(user.lastname, res, row_id, cols_id[user.lastname.n]);
  load_field_from_res(user.student_id, res, row_id, cols_id[user.student_id.n]);
  load_field_from_res(user.username, res, row_id, cols_id[user.username.n]);
  load_field_from_res(user.email, res, row_id, cols_id[user.email.n]);
  load_field_from_res(user.role, res, row_id, cols_id[user.role.n]);
  return user;
}

} // namespace Sphinx::Db
