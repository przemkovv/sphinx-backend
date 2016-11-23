#include "model_utils.h"

namespace Sphinx::Db {

template <>
QueryParams to_insert_params(const Backend::Model::User &data)
{
  return make_value_list(data.username, data.email);
}

//----------------------------------------------------------------------
template <>
QueryParams to_insert_params(const Backend::Model::Course &data)
{
  return make_value_list(data.name, data.description);
}

//----------------------------------------------------------------------
template <>
QueryParams to_insert_params(const Backend::Model::Module &data)
{
  return make_value_list(data.course_id, data.name, data.description);
}

template <>
Db::Meta::ColumnsId<Backend::Model::Module> get_columns_id(PGresult *res)
{
  return {PQfnumber(res, Backend::Model::Module::Columns::id_n),
          PQfnumber(res, Backend::Model::Module::Columns::course_id_n),
          PQfnumber(res, Backend::Model::Module::Columns::name_n),
          PQfnumber(res, Backend::Model::Module::Columns::description_n)};
}

//----------------------------------------------------------------------
template <>
Db::Meta::ColumnsId<Backend::Model::Course> get_columns_id(PGresult *res)
{
  return {PQfnumber(res, Backend::Model::Course::Columns::id_n),
          PQfnumber(res, Backend::Model::Course::Columns::name_n),
          PQfnumber(res, Backend::Model::Course::Columns::description_n)};
}

//----------------------------------------------------------------------
template <>
Db::Meta::ColumnsId<Backend::Model::User> get_columns_id(PGresult *res)
{
  return {PQfnumber(res, Backend::Model::User::Columns::id_n),
          PQfnumber(res, Backend::Model::User::Columns::username_n),
          PQfnumber(res, Backend::Model::User::Columns::email_n)};
}

//----------------------------------------------------------------------
template <>
Backend::Model::Course get_row<Backend::Model::Course>(
    PGresult *res,
    const Db::Meta::ColumnsId<Backend::Model::Course> &cols_id,
    int row_id)
{
  Backend::Model::Course course;
  load_field_from_res(course.id, res, row_id, cols_id.id);
  load_field_from_res(course.name, res, row_id, cols_id.name);
  load_field_from_res(course.description, res, row_id, cols_id.description);
  return course;
}

//----------------------------------------------------------------------
template <>
Backend::Model::Module get_row<Backend::Model::Module>(
    PGresult *res,
    const Db::Meta::ColumnsId<Backend::Model::Module> &cols_id,
    int row_id)
{
  Backend::Model::Module module;
  load_field_from_res(module.id, res, row_id, cols_id.id);
  load_field_from_res(module.name, res, row_id, cols_id.name);
  load_field_from_res(module.description, res, row_id, cols_id.description);
  load_field_from_res(module.course_id, res, row_id, cols_id.course_id);
  return module;
}
//----------------------------------------------------------------------
template <>
Backend::Model::User get_row<Backend::Model::User>(
    PGresult *res,
    const Db::Meta::ColumnsId<Backend::Model::User> &cols_id,
    int row_id)
{
  Backend::Model::User user;
  load_field_from_res(user.id, res, row_id, cols_id.id);
  load_field_from_res(user.username, res, row_id, cols_id.username);
  load_field_from_res(user.email, res, row_id, cols_id.email);
  return user;
}

} // namespace Sphinx::Db
