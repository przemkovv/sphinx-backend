
#include "dao.h"
#include <exception>       // for exception
#include <memory>          // for __shared_ptr_access
#include <spdlog/spdlog.h> // for logger

//----------------------------------------------------------------------
namespace Sphinx::Backend::Db {

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
DAO::get_modules(Meta::IdColumn_t<Model::Course> /* course_id */)
{
  return db_connection_.get_all<Model::Module>();
}

//----------------------------------------------------------------------
void DAO::create_course(const Model::Course &course)
{
  auto last_id = db_connection_.insert(course);
  logger_->debug("Created course with last id: {}", last_id);
}

//----------------------------------------------------------------------
void DAO::create_module(const Model::Module &module)
{
  auto last_id = db_connection_.insert(module);
  logger_->debug("Created module with last id: {}", last_id);
}

//----------------------------------------------------------------------
void DAO::create_user(const Model::User &user)
{
  auto last_id = db_connection_.insert(user);
  logger_->debug("Created user with last id: {}", last_id);
}

} // namespace Sphinx::Backend::Db
