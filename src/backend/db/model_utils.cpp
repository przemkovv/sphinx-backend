#include "model_utils.h"

namespace Sphinx::Db {

template <>
Meta::ColumnsId<Backend::Model::Module> get_columns_id(PGresult *res)
{
  using E = Backend::Model::Module;
  using Columns = decltype(std::declval<E>().get_columns());
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
          PQfnumber(res, decltype(E::title)::name),
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
  load_fields_from_res(res, row_id, cols_id, course.id, course.title,
                       course.description);
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
  load_fields_from_res(res, row_id, cols_id, module.id, module.name,
                       module.description, module.course_id);
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
  load_fields_from_res(res, row_id, cols_id, user.id, user.firstname,
                       user.lastname, user.student_id, user.username,
                       user.email, user.role);
  return user;
}

} // namespace Sphinx::Db
