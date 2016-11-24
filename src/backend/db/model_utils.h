#pragma once

#include "model.h"
#include "db_utils.h"

namespace Sphinx::Db {

template <>
Meta::ColumnsId<Backend::Model::Module> get_columns_id(PGresult *res);

//----------------------------------------------------------------------
template <>
Meta::ColumnsId<Backend::Model::Course> get_columns_id(PGresult *res);

//----------------------------------------------------------------------
template <>
Meta::ColumnsId<Backend::Model::User> get_columns_id(PGresult *res);

//----------------------------------------------------------------------
template <>
Backend::Model::Course get_row<Backend::Model::Course>(
    PGresult *res,
    const Meta::ColumnsId<Backend::Model::Course> &cols_id,
    int row_id);

//----------------------------------------------------------------------
template <>
Backend::Model::Module get_row<Backend::Model::Module>(
    PGresult *res,
    const Meta::ColumnsId<Backend::Model::Module> &cols_id,
    int row_id);
//----------------------------------------------------------------------
template <>
Backend::Model::User get_row<Backend::Model::User>(
    PGresult *res,
    const Meta::ColumnsId<Backend::Model::User> &cols_id,
    int row_id);

} // namespace Sphinx::Db
