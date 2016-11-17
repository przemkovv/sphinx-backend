
#include "BackendApp.h"

#include "db/json_serializer.h"

#include <utils.h>

#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

#include <sphinx_assert.h>

using std::literals::operator""s;

namespace Sphinx::Backend {

BackendApp::BackendApp(const std::string &application_name,
                       const std::vector<std::string> &args)
  : Application(application_name, args),
    db_config_(prepare_db_config()),
    db_(db_config_),
    REST_API_PORT(config_get<std::uint16_t>(REST_API_PORT_PATH)),
    REST_API_VERSION("/"s + config_get<std::string>(REST_API_VERSION_PATH)),
    dump_indent_(config_get<int>(DUMP_LEVEL_PATH))

{
}

Db::connection_config BackendApp::prepare_db_config()
{
  const auto db_engine_path = "/database/engine"_json_pointer;
  if (auto engine = config_get<std::string>(db_engine_path);
      engine != "postgres") {
    throw std::invalid_argument("Unsupported database engine.");
  }
  else {
    const auto db_host_path = "/database/host"_json_pointer;
    const auto db_port_path = "/database/port"_json_pointer;
    const auto db_password_path = "/database/password"_json_pointer;
    const auto db_user_path = "/database/user"_json_pointer;
    const auto db_dbname_path = "/database/dbname"_json_pointer;

    Db::connection_config config;
    config.host = config_get<std::string>(db_host_path);
    config.port = config_get<std::uint16_t>(db_port_path);
    config.user = config_get<std::string>(db_user_path);
    config.password = config_get<std::string>(db_password_path);
    config.db_name = config_get<std::string>(db_dbname_path);

    return config;
  }
}

std::string BackendApp::get_users()
{
  return db_.get_users_json().dump(dump_indent_);
}

void BackendApp::create_user(const std::string &data) {

  auto json_data = nlohmann::json::parse(data);
  logger()->debug("JSON: {} {}", data, json_data.dump(dump_indent_));

  auto user = Sphinx::Db::from_json<Sphinx::Db::User>(json_data);

  logger()->debug("Creating user {} {}", user.username, user.email);
  SPHINX_ASSERT(false, "not implemented");
  // db_.create_user(user);

}
int BackendApp::run()
{
  logger()->debug("Configuration file: {}", config().dump(dump_indent_));

  add_route("/users", "GET"_method,
            "POST"_method)([&](const crow::request &req) {
    if (req.method == "GET"_method) {
      return get_users();
    }
    else if (req.method == "POST"_method) {
      logger()->debug("POST body {}", req.body);
      create_user(req.body);
    }
    return "nothing"s;
  });

  app_.port(REST_API_PORT).run();

  return static_cast<int>(ExitCode::OK);
}

} // namespace Sphinx::Backend
