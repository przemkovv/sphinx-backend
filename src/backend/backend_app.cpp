
#include "backend_app.h"

#include "db/json_serializer.h"

#include "utils.h"

#include <iostream>
#include <stdexcept>
#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "sphinx_assert.h"

using namespace std::literals::string_literals;

namespace Sphinx::Backend {

using Sphinx::Db::Json::to_json;
using Sphinx::Db::Json::from_json;
using Sphinx::Db::Meta::IdColumn;

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
std::string BackendApp::get_modules(Meta::IdColumn_t<Model::Course> course_id)
{
  return to_json(dao_.get_modules(course_id)).dump(dump_indent_);
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

std::string BackendApp::find_users(std::string name)
{
  Model::User user;
  return to_json(dao_.find_by_column(user.username, name)).dump(dump_indent_);
}
//----------------------------------------------------------------------
void BackendApp::add_users_routes()
{
  add_route("/users_search/<string>", "GET"_method,
            "POST"_method)([&](const crow::request &req, std::string name) {
    if (req.method == "GET"_method) {
      return find_users(name);
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

  add_route("/users/<uint>/exists", "GET"_method,
            "PUT"_method)([&](const crow::request &req, std::uint64_t user_id) {
    if (req.method == "GET"_method) {
      auto entity = is_entity_exists<Model::User>(user_id);
      return to_json(entity).dump(dump_indent_);
    }
    return "nothing"s;
  });

  add_route("/users/<uint>", "GET"_method,
            "PUT"_method)([&](const crow::request &req, std::uint64_t user_id) {
    if (req.method == "GET"_method) {
      auto entity = get_entity<Model::User>(user_id);
      return to_json(entity).dump(dump_indent_);
    }
    else if (req.method == "PUT"_method) {
      logger()->debug("PUT body {}", req.body);
      auto entity = update_entity<Model::User>(user_id, req.body);
      return to_json(entity).dump(dump_indent_);
    }
    return "nothing"s;
  });
}

//----------------------------------------------------------------------
void BackendApp::add_courses_routes()
{
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
}

//----------------------------------------------------------------------
void BackendApp::add_modules_routes()
{
  add_route("/courses/<uint>", "GET"_method, "POST"_method)(
      [&](const crow::request &req, std::uint64_t course_id) {
        if (req.method == "GET"_method) {
          return get_modules(course_id);
        }
        else if (req.method == "POST"_method) {
          logger()->debug("POST body {}", req.body);
          create_entities<Model::Module>(req.body);
        }
        return "nothing"s;
      });
}

//----------------------------------------------------------------------
void BackendApp::add_test_routes()
{
  add_route("/test", "GET"_method,
            "POST"_method)([&](const crow::request &req) {
    if (req.method == "GET"_method) {
      logger()->debug("GET body {}", req.body);
      logger()->debug("Query string: {}", req.url_params);
      logger()->debug("Query Id: {}", req.url_params.get("id"));
      fmt::MemoryWriter w;
      for (const auto &header : req.headers) {
        w.write("{{{}: {}}}, ", header.first, header.second);
      }
      logger()->debug("Headers: {}", w.str());
      return "GET"s;
    }
    else if (req.method == "POST"_method) {
      logger()->debug("POST body {}", req.body);
      return "POST"s;
    }
    return "nothing"s;
  });
}

//----------------------------------------------------------------------
int BackendApp::run()
{
  logger()->debug("Configuration file: {}", config().dump(dump_indent_));

  add_users_routes();
  add_courses_routes();
  add_modules_routes();
  add_test_routes();

  app_.port(REST_API_PORT).run();

  return static_cast<int>(ExitCode::OK);
}

} // namespace Sphinx::Backend
