
#include "BackendApp.h"

#include <utils.h>

#include <iostream>
#include <json_diag.hpp>
#include <stdexcept>
#include <string>

using std::literals::operator""s;

namespace Sphinx::Backend {

BackendApp::BackendApp(const std::string &application_name,
                       const std::vector<std::string> &args)
  : Application(application_name, args),
    db_config_(prepare_db_config()),
    db_(db_config_),
    REST_API_PORT(config<std::uint16_t>(REST_API_PORT_PATH)),
    REST_API_VERSION("/"s + config<std::string>(REST_API_VERSION_PATH))

{
}

Db::connection_config BackendApp::prepare_db_config()
{
  const auto db_engine_path = "/database/engine"_json_pointer;
  if (auto engine = config<std::string>(db_engine_path); engine != "sqlite3") {
    throw std::invalid_argument("Unsupported database engine.");
  }
  else {
    const auto db_path_path = "/database/path"_json_pointer;
    const auto db_debug_path = "/database/debug"_json_pointer;
    const auto db_password_path = "/database/password"_json_pointer;

    return {config<std::string>(db_path_path), SQLITE_OPEN_READWRITE, "",
            config<bool>(db_debug_path), config<std::string>(db_password_path)};
  }
}

std::string BackendApp::get_users()
{
  const auto users = Sphinx::Db::get_users(db_);
  const auto response = Sphinx::Db::to_json(users);
  return response.dump();
}

int BackendApp::run()
{
  logger()->debug("Configuration file: {}", config().dump(2));

  add_route("/users", "GET"_method,
            "POST"_method)([&](const crow::request &req) {
    if (req.method == "GET"_method) {
      return get_users();
    }
    return "nothing"s;
  });

  app_.port(REST_API_PORT).run();

  return static_cast<int>(ExitCode::OK);
}

} // namespace Sphinx::Backend
