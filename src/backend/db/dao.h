
#pragma once

#include "db/model.h"
#include "db_connection.h" // for DbConnection, connection_config
#include "logger.h"        // for make_logger, Logger
#include "model_expr.h"
#include "model_meta.h" // for IdColumn, IdColumn_t, Meta
#include <algorithm>    // for move
#include <optional>     // for optional
#include <vector>       // for vector

namespace Sphinx::Backend::Db {

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
  //----------------------------------------------------------------------
  template <typename Column>
  std::vector<typename Column::entity>
  find_by_column(const Column &column, const typename Column::type &value)
  {
    auto condition =
        Sphinx::Db::condition{column, Sphinx::Db::eq_operator{}, value};
    return db_connection_.get_all_where<typename Column::entity>(condition);
  }
  //----------------------------------------------------------------------
  template <typename T>
  std::optional<T> find_by_id(typename T::Columns::id_column_t id)
  {
    return db_connection_.find_by_id<T>(id);
  }
  //----------------------------------------------------------------------
  template <typename T>
  bool exists(typename Sphinx::Db::Meta::IdColumn<T>::type id)
  {
    return db_connection_.exists<T>(id);
  }
  //----------------------------------------------------------------------
  template <typename T>
  T get_by_id(typename Sphinx::Db::Meta::IdColumn<T>::type id)
  {
    return db_connection_.get_by_id<T>(id);
  }
  //----------------------------------------------------------------------
  std::vector<Model::User> get_users();
  std::vector<Model::Course> get_courses();
  std::vector<Model::Module> get_modules();
  std::vector<Model::Module>
  get_modules(Meta::IdColumn_t<Model::Course> course_id);
  Meta::IdColumn_t<Model::User> create_user(const Model::User &user);
  Meta::IdColumn_t<Model::Course> create_course(const Model::Course &course);
  Meta::IdColumn_t<Model::Module> create_module(const Model::Module &module);

private:
  Logger logger_;
};
} // namespace Sphinx::Backend::Db

//----------------------------------------------------------------------
