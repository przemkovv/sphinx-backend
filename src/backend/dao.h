
#pragma once

#include "db/db_connection.h" // for DbConnection, connection_config
#include "db/model_expr.h"
#include "db/model_meta.h" // for IdColumn, IdColumnType, Meta
#include "model/model.h"
#include "shared_lib/logger.h" // for make_logger, Logger
#include <algorithm>           // for move
#include <optional>            // for optional
#include <vector>              // for vector

namespace Sphinx::Backend {

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
  auto find_by_column(const Column &column, const typename Column::type &value)
  {
    auto condition =
        Sphinx::Db::condition{column, Sphinx::Db::eq_operator{}, value};
    return db_connection_.get_all_where<typename Column::entity>(condition);
  }
  //----------------------------------------------------------------------
  template <typename T>
  std::optional<T> find_by_id(typename Meta::IdColumnType<T> id);

  template <typename T>
  bool exists(Meta::IdColumnType<T> id);

  template <typename T>
  T get_by_id(Meta::IdColumnType<T> id);

  //----------------------------------------------------------------------
  std::vector<Model::User> get_users();
  std::vector<Model::Course> get_courses();
  std::vector<Model::Module> get_modules();
  std::vector<Model::Module>
  get_modules(Meta::IdColumnType<Model::Course> course_id);
  Meta::IdColumnType<Model::User> create_user(const Model::User &user);
  Meta::IdColumnType<Model::Course> create_course(const Model::Course &course);
  Meta::IdColumnType<Model::Module> create_module(const Model::Module &module);

private:
  Logger logger_;
};

} // namespace Sphinx::Backend

//----------------------------------------------------------------------
