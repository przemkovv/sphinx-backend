
#pragma once

#include "db.h"

#include "model.h"
#include "model_utils.h"

#include <map>
#include <string>
#include <vector>

namespace Sphinx::Backend::Db {

class BackendDb : public Sphinx::Db::DbConnection {
public:
  BackendDb(Sphinx::Db::connection_config config)
    : DbConnection(std::move(config))
  {
    for (const auto &table : {std::string{Model::User::Table::name},
                              std::string{Model::Module::Table::name},
                              std::string{Model::Course::Table::name}}) {
      auto name = fmt::format("{}", table);
      auto query = fmt::format("SELECT * FROM {}", table);
      prepared_statements.emplace(name, prepare(name, query, 0));
    }
  }

private:
  std::map<std::string, Sphinx::Db::prepared_statement> prepared_statements;

public:
  std::vector<Model::User> get_users();
  std::vector<Model::Course> get_courses();
  std::vector<Model::Module> get_modules();
};
} // namespace Sphinx::Backend::Db

//----------------------------------------------------------------------
