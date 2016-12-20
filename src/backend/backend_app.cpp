
#include "backend_app.h"

#include "json/json_serializer.h"

#include "shared_lib/utils.h"

#include <iostream>
#include <stdexcept>
#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "shared_lib/sphinx_assert.h"

using namespace std::literals::string_literals;

namespace Sphinx::Backend {


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
Model::Courses BackendApp::get_courses()
{
  return dao_.get_courses();
}

//----------------------------------------------------------------------
Model::Modules BackendApp::get_modules()
{
  return dao_.get_modules();
}

//----------------------------------------------------------------------
Model::Modules
BackendApp::get_modules(Meta::IdColumnType<Model::Course> course_id)
{
  return dao_.get_modules(course_id);
}

//----------------------------------------------------------------------
Model::Users BackendApp::get_users()
{
  return dao_.get_users();
}

//----------------------------------------------------------------------
template <>
Meta::IdColumnType<Model::Module>
BackendApp::create_entity(const Model::Module &module)
{
  logger()->debug("Creating module {} {} {}", module.course_id.value,
                  module.title.value,
                  module.description.value.value_or("EMPTY"));
  return dao_.create_module(module);
}

//----------------------------------------------------------------------
template <>
Meta::IdColumnType<Model::Course>
BackendApp::create_entity(const Model::Course &course)
{
  logger()->debug("Creating course {} {}", course.title.value,
                  course.description.value.value_or("EMPTY"));
  return dao_.create_course(course);
}

//----------------------------------------------------------------------
template <>
Meta::IdColumnType<Model::User>
BackendApp::create_entity(const Model::User &user)
{
  logger()->debug("Creating user {} {}", user.username.value, user.email.value);
  return dao_.create_user(user);
}

//----------------------------------------------------------------------
Model::Users BackendApp::find_users(std::string name)
{
  Model::User user;
  return dao_.find_by_column(user.username, name);
}

//----------------------------------------------------------------------
void BackendApp::add_users_routes()
{
  add_route("/users_search/<string>", "GET"_method,
            "POST"_method)([&](const crow::request &req, std::string name) {
    if (req.method == "GET"_method) {
      const auto users = find_users(name);
      if (users.size())
        return response(201, users);
      else
        return response(204);
    }
    return response(404);
  });

  add_route("/users", "GET"_method,
            "POST"_method)([&](const crow::request &req) {
    if (req.method == "GET"_method) {
      return response(200, get_users());
    }
    else if (req.method == "POST"_method) {
      logger()->debug("POST body {}", req.body);
      auto r = create_entities<Model::User>(req.body);
      return response(r);
    }
    return response(404);
  });

  add_route("/users/<int>/exists", "GET"_method,
            "PUT"_method)([&](const crow::request &req, std::int32_t user_id) {
    if (req.method == "GET"_method) {
      auto exists = is_entity_exists<Model::User>(user_id);
      return response(200, exists);
    }
    return response(404);
  });

  add_route("/users/<int>", "GET"_method,
            "PUT"_method)([&](const crow::request &req, std::int32_t user_id) {
    if (req.method == "GET"_method) {
      auto entity = get_entity<Model::User>(user_id);
      return response(200, entity);
    }
    else if (req.method == "PUT"_method) {
      logger()->debug("PUT body {}", req.body);
      auto entity = update_entity<Model::User>(user_id, req.body);
      return response(200, entity);
    }
    return response(404);
  });
}

//----------------------------------------------------------------------
void BackendApp::add_courses_routes()
{
  add_route("/courses", "GET"_method,
            "POST"_method)([&](const crow::request &req) {
    if (req.method == "GET"_method) {
      return response(200, get_courses());
    }
    else if (req.method == "POST"_method) {
      logger()->debug("POST body {}", req.body);
      auto entities = deserialize_entities<Model::Course>(req.body, true);
      create_entities<Model::Course>(entities);
      return response(201, entities);
    }
    return response(404);
  });
}

//----------------------------------------------------------------------
void BackendApp::add_modules_routes()
{
  add_route("/courses/<int>", "GET"_method, "POST"_method)(
      [&](const crow::request &req, std::int32_t course_id) {
        if (req.method == "GET"_method) {
          return response(200, get_modules(course_id));
        }
        else if (req.method == "POST"_method) {
          logger()->debug("POST body {}", req.body);
          create_entities<Model::Module>(req.body);
          return response(201);
        }
        return response(404);
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
      return response(501);
    }
    else if (req.method == "POST"_method) {
      logger()->debug("POST body {}", req.body);
      return response(501);
    }
    return response(404);
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
