
#include "BackendApp.h"

#include "db/json_serializer.h"

#include "utils.h"

#include <iostream>
#include <stdexcept>
#include <string>

#include "sphinx_assert.h"

using namespace std::literals::string_literals;

namespace Sphinx::Backend {

using Sphinx::Db::Json::to_json;
using Sphinx::Db::Json::from_json;

//----------------------------------------------------------------------
BackendApp::BackendApp(const std::string &application_name,
                       const std::vector<std::string> &args)
  : Application(application_name, args),
    dao_(prepare_db_config()),
    REST_API_PORT(config_get<std::uint16_t>(REST_API_PORT_PATH)),
    REST_API_VERSION("/"s + config_get<std::string>(REST_API_VERSION_PATH)),
    dump_indent_(config_get<int>(DUMP_LEVEL_PATH))

{
}

//----------------------------------------------------------------------
Sphinx::Db::connection_config BackendApp::prepare_db_config()
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

    Sphinx::Db::connection_config config;
    config.host = config_get<std::string>(db_host_path);
    config.port = config_get<std::uint16_t>(db_port_path);
    config.user = config_get<std::string>(db_user_path);
    config.password = config_get<std::string>(db_password_path);
    config.db_name = config_get<std::string>(db_dbname_path);

    return config;
  }
}

//----------------------------------------------------------------------
std::string BackendApp::get_courses()
{
  return to_json(dao_.get_courses()).dump(dump_indent_);
}

//----------------------------------------------------------------------
std::string BackendApp::get_modules()
{
  return to_json(dao_.get_modules()).dump(dump_indent_);
}

//----------------------------------------------------------------------
std::string BackendApp::get_users()
{
  return to_json(dao_.get_users()).dump(dump_indent_);
}

//----------------------------------------------------------------------
template <>
void BackendApp::create_entity<Model::Module>(const nlohmann::json &data)
{
  auto module = from_json<Model::Module>(data);
  logger()->debug("Creating course {} {} {}", module.course_id.value,
                  module.name.value,
                  module.description.value.value_or("EMPTY"));
  dao_.create_module(module);
}

//----------------------------------------------------------------------
template <>
void BackendApp::create_entity<Model::Course>(const nlohmann::json &data)
{
  auto course = from_json<Model::Course>(data);
  logger()->debug("Creating course {} {}", course.name.value,
                  course.description.value.value_or("EMPTY"));
  dao_.create_course(course);
}

//----------------------------------------------------------------------
template <>
void BackendApp::create_entity<Model::User>(const nlohmann::json &data)
{
  auto user = from_json<Model::User>(data);
  logger()->debug("Creating user {} {}", user.username.value, user.email.value);
  dao_.create_user(user);
}

//----------------------------------------------------------------------
int BackendApp::run()
{
  logger()->debug("Configuration file: {}", config().dump(dump_indent_));

  add_route("/courses", "GET"_method,
            "POST"_method)([&](const crow::request &req) {
    if (req.method == "GET"_method) {
      return get_courses();
    }
    else if (req.method == "POST"_method) {
      logger()->debug("POST body {}", req.body);
      create_entities<Model::Course>(req.body);
    }
    return "nothing"s;
  });

  add_route("/modules", "GET"_method,
            "POST"_method)([&](const crow::request &req) {
    if (req.method == "GET"_method) {
      return get_modules();
    }
    else if (req.method == "POST"_method) {
      logger()->debug("POST body {}", req.body);
      create_entities<Model::Module>(req.body);
    }
    return "nothing"s;
  });

  add_route("/users", "GET"_method,
            "POST"_method)([&](const crow::request &req) {
    if (req.method == "GET"_method) {
      return get_users();
    }
    else if (req.method == "POST"_method) {
      logger()->debug("POST body {}", req.body);
      create_entities<Model::User>(req.body);
    }
    return "nothing"s;
  });

  app_.port(REST_API_PORT).run();

  return static_cast<int>(ExitCode::OK);
}

} // namespace Sphinx::Backend
