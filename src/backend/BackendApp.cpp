
#include "BackendApp.h"

#include "db/json_serializer.h"

#include <utils.h>

#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

using std::literals::operator""s;

namespace Sphinx::Backend {

BackendApp::BackendApp(const std::string &application_name,
                       const std::vector<std::string> &args)
  : Application(application_name, args),
    db_config_(prepare_db_config()),
    db_(db_config_),
    REST_API_PORT(config_get<std::uint16_t>(REST_API_PORT_PATH)),
    REST_API_VERSION("/"s + config_get<std::string>(REST_API_VERSION_PATH))

{
}

Db::connection_config BackendApp::prepare_db_config()
{
  const auto db_engine_path = "/database/engine"_json_pointer;
  if (auto engine = config_get<std::string>(db_engine_path);
      engine != "sqlite3") {
    throw std::invalid_argument("Unsupported database engine.");
  }
  else {
    const auto db_path_path = "/database/path"_json_pointer;
    const auto db_debug_path = "/database/debug"_json_pointer;
    const auto db_password_path = "/database/password"_json_pointer;

    return {config_get<std::string>(db_path_path), SQLITE_OPEN_READWRITE, "",
            config_get<bool>(db_debug_path),
            config_get<std::string>(db_password_path)};
  }
}

std::string BackendApp::get_users()
{
  return db_.get_users_json().dump(2);
  // const auto users = db_.get_users();
  // const auto response = Db::to_json(users);
  // return response.dump();
}

void BackendApp::create_user(const std::string &data) {
  auto json_data = nlohmann::json::parse(data);
  logger()->debug("JSON: {} {}", data, json_data.dump(1));

  auto user = Sphinx::Db::from_json<Sphinx::Db::User>(data);

  logger()->debug("Creating user {} {}", user.username, user.email);

}
int BackendApp::run()
{
  logger()->debug("Configuration file: {}", config().dump(2));

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
