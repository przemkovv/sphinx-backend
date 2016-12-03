
#pragma once

#include "Logger.h"        // for make_logger, Logger
#include "db_connection.h" // for DbConnection, connection_config
#include "model.h"         // for Module, Course, User
#include "model_expr.h"
#include "model_utils.h"
#include <algorithm> // for move
#include <experimental/optional>
#include <vector> // for vector

namespace Sphinx::Backend::Db {

using std::experimental::optional;
using std::experimental::nullopt;

namespace Meta = Sphinx::Db::Meta;

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
  template <typename Column>
  std::vector<typename Column::entity>
  find_by_column(const Column &column, const typename Column::type &value)
  {
    auto condition =
        Sphinx::Db::condition{column, Sphinx::Db::eq_operator{}, value};
    return db_connection_.get_all_where<typename Column::entity>(condition);
  }
  template <typename T>
  optional<T> find_by_id(typename T::Columns::id_column_t id)
  {
    return db_connection_.find_by_id<T>(id);
  }
  template <typename T>
  bool exists(typename Sphinx::Db::Meta::IdColumn<T>::type id)
  {
    return db_connection_.exists<T>(id);
  }
  template <typename T>
  T get_by_id(typename Sphinx::Db::Meta::IdColumn<T>::type id)
  {
    return db_connection_.get_by_id<T>(id);
  }
  std::vector<Model::User> get_users();
  std::vector<Model::Course> get_courses();
  std::vector<Model::Module> get_modules();
  std::vector<Model::Module>
  get_modules(Meta::IdColumn_t<Model::Course> course_id);
  void create_user(const Model::User &user);
  void create_course(const Model::Course &course);
  void create_module(const Model::Module &module);

private:
  Logger logger_;
};
} // namespace Sphinx::Backend::Db

//----------------------------------------------------------------------
