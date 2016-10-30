
#include "BackendApp.h"

// #include <utils.h>

#include <iostream>
#include <string>

using std::literals::operator""s;

namespace Sphinx::Backend {

BackendApp::BackendApp(const std::string &application_name,
                       const std::vector<std::string> &args)
  : Application(application_name, args),
    db_config_(prepare_db_config()),
    db_(db_config_)
{
}

Db::connection_config BackendApp::prepare_db_config()
{
  Db::connection_config config{};
  config.path_to_database =
      "/home/przemkovv/projects/sphinx/backend/data/sphinx_backend.sqlite";
  config.flags = SQLITE_OPEN_READWRITE;
  config.debug = true;
  return config;
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

  auto logger = Sphinx::make_logger("Sphinx::Backend");

  add_route("/users", "GET"_method,
            "POST"_method)([&](const crow::request &req) {
    if (req.method == "GET"_method) {
      return get_users();
    }
    return "nothing"s;
  });

  app_.port(SPHINX_BACKEND_PORT).run();

  return 0;
  // return static_cast<int>(ExitCode::OK);
}

} // namespace Sphinx::Backend
