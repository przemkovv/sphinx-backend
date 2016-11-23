
#pragma once

#include "Logger.h"        // for make_logger, Logger
#include "db_connection.h" // for DbConnection, connection_config
#include "model.h"         // for Module, Course, User
#include "model_utils.h"
#include <algorithm> // for move
#include <experimental/optional>
#include <vector> // for vector

namespace Sphinx::Backend::Db {

using std::experimental::optional;
using std::experimental::nullopt;

class DAO {
public:
  DAO(Sphinx::Db::connection_config config)
    : db_connection_(std::move(config)),
      logger_(Sphinx::make_logger("Sphinx::Backend::Db::DAO"))
  {
  }

private:
  Sphinx::Db::DbConnection db_connection_;

public:
  template <typename T>
  optional<T> find_by_column(typename T::Columns::id_column_t id)
  {
    not_implemented_yet<T>();
  }
  template <typename T>
  optional<T> find_by_id(typename T::Columns::id_column_t id)
  {
    not_implemented_yet<T>();
  }
  template <typename T>
  T get_by_id(typename T::Columns::id_column_t id)
  {
    not_implemented_yet<T>();
  }
  std::vector<Model::User> get_users();
  std::vector<Model::Course> get_courses();
  std::vector<Model::Module> get_modules();
  void create_user(const Model::User &user);
  void create_course(const Model::Course &course);
  void create_module(const Model::Module &module);

private:
  Logger logger_;
};
} // namespace Sphinx::Backend::Db

//----------------------------------------------------------------------
