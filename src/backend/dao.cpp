
#include "dao.h"
#include "model/model.h"      // for User, Module, Course
#include <memory>          // for __shared_ptr_access
#include <spdlog/spdlog.h> // for logger

#include <boost/hana/core/to.hpp>  // for to_t::operator()

//----------------------------------------------------------------------
namespace Sphinx::Backend {

std::vector<Model::User> DAO::get_users()
{
  return db_connection_.get_all<Model::User>();
}

//----------------------------------------------------------------------
std::vector<Model::Course> DAO::get_courses()
{
  return db_connection_.get_all<Model::Course>();
}

//----------------------------------------------------------------------
std::vector<Model::Module> DAO::get_modules()
{
  return db_connection_.get_all<Model::Module>();
}

//----------------------------------------------------------------------
std::vector<Model::Module>
    DAO::get_modules(Meta::IdColumnType<Model::Course> /* course_id */)
{
  return db_connection_.get_all<Model::Module>();
}

//----------------------------------------------------------------------
Db::Meta::IdColumnType<Model::Course>
DAO::create_course(const Model::Course &course)
{
  auto last_id = db_connection_.insert(course);
  logger_->debug("Created course with last id: {}", last_id);
  return last_id;
}

//----------------------------------------------------------------------
Db::Meta::IdColumnType<Model::Module>
DAO::create_module(const Model::Module &module)
{
  auto last_id = db_connection_.insert(module);
  logger_->debug("Created module with last id: {}", last_id);
  return last_id;
}

//----------------------------------------------------------------------
Db::Meta::IdColumnType<Model::User>
DAO::create_user(const Model::User &user)
{
  auto last_id = db_connection_.insert(user);
  logger_->debug("Created user with last id: {}", last_id);
  return last_id;
}

} // namespace Sphinx::Backend
