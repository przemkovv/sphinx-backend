
#pragma once

#include "data.h"

#include <fmt/format.h>
#include <Logger.h>
#include <libpq-fe.h>
#include <nlohmann/json.hpp>

#include <vector>

#include <experimental/propagate_const>
#include <memory>

namespace Sphinx::Db {

struct connection_config {
  std::string host;
  std::uint16_t port;
  std::string user;
  std::string password;
  std::string db_name;

  std::string get_connection_string() const
  {
    return fmt::format("postgresql://{0}:{1}@{2}:{3}/{4}", user, password,
                       host, port, db_name);
  }
};


class Db {

public:
  Db(const connection_config &db_config);
  ~Db();

private:
  const connection_config db_config_;

  PGconn *conn;

public:
  // std::vector<User> get_users();
  // nlohmann::json get_users_json();
  // void create_user(const User &user);

private:
  // struct Queries;
  // std::experimental::propagate_const<std::unique_ptr<Queries>> queries_;


  Logger logger_;
};

} // namespace Sphinx::Db
