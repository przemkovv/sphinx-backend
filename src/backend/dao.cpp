
#include "dao.h"
#include "model/model.h"   // for User, Module, Course
#include <memory>          // for __shared_ptr_access
#include <spdlog/spdlog.h> // for logger

#include <boost/hana/core/to.hpp> // for to_t::operator()

//----------------------------------------------------------------------
namespace Sphinx::Backend {

template std::optional<Model::User>
DAO::find_by_id(Meta::IdColumnType<Model::User> id);
template std::optional<Model::Module>
DAO::find_by_id(Meta::IdColumnType<Model::Module> id);
template std::optional<Model::Course>
DAO::find_by_id(Meta::IdColumnType<Model::Course> id);

template bool DAO::exists<Model::User>(Meta::IdColumnType<Model::User> id);
template bool DAO::exists<Model::Module>(Meta::IdColumnType<Model::Module> id);
template bool DAO::exists<Model::Course>(Meta::IdColumnType<Model::Course> id);
template Model::User DAO::get_by_id(Meta::IdColumnType<Model::User> id);
template Model::Module DAO::get_by_id(Meta::IdColumnType<Model::Module> id);
template Model::Course DAO::get_by_id(Meta::IdColumnType<Model::Course> id);

//----------------------------------------------------------------------
DAO::DAO(Sphinx::Db::connection_config config)
  : db_connection_(std::move(config)),
    logger_(Sphinx::make_logger("Sphinx::Backend::Db::DAO"))
{
}
//----------------------------------------------------------------------
template <typename T>
std::optional<T> DAO::find_by_id(typename Meta::IdColumnType<T> id)
{
  return db_connection_.find_by_id<T>(id);
}
//----------------------------------------------------------------------
template <typename T>
bool DAO::exists(Meta::IdColumnType<T> id)
{
  return db_connection_.exists<T>(id);
}
//----------------------------------------------------------------------
template <typename T>
T DAO::get_by_id(Meta::IdColumnType<T> id)
{
  return db_connection_.get_by_id<T>(id);
}
//----------------------------------------------------------------------
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
DAO::get_modules(Meta::IdColumnType<Model::Course> course_id)
{
  Model::Module module;
  auto condition = Sphinx::Db::condition{module.course_id,
                                         Sphinx::Db::eq_operator{}, course_id};
  return db_connection_.get_all_where<Model::Module>(condition);
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
Db::Meta::IdColumnType<Model::User> DAO::create_user(const Model::User &user)
{
  auto last_id = db_connection_.insert(user);
  logger_->debug("Created user with last id: {}", last_id);
  return last_id;
}

} // namespace Sphinx::Backend
