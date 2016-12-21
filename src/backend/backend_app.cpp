
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
crow::response BackendApp::response(int code)
{
  return crow::response(code);
}

//----------------------------------------------------------------------
crow::response BackendApp::response(int code, const Header &header)
{
  auto r = crow::response(code);
  r.set_header(header.name, header.value);
  return r;
}

//----------------------------------------------------------------------
crow::response BackendApp::response(int code, const std::string &body)
{
  return crow::response(code, body);
}

//----------------------------------------------------------------------
crow::response BackendApp::response(int code, const nlohmann::json &body)
{
  auto r = crow::response(code, body.dump(dump_indent_));
  r.set_header("Content-Type", "application/json");
  return r;
}

//----------------------------------------------------------------------
template <typename T>
crow::response BackendApp::response(int code, const T &data)
{
  const auto body = Json::to_json(data);
  return response(code, body);
}

//----------------------------------------------------------------------
crow::response BackendApp::response(const Responses &r)
{
  return response(merge_responses(r));
}

//----------------------------------------------------------------------
crow::response BackendApp::response(const Response &r)
{
  auto cr = crow::response(static_cast<int>(r.status));
  if (r.body)
    cr.body = r.body->dump(dump_indent_);
  for (const auto &header : r.headers) {
    cr.set_header(header.name, header.value);
  }
  return cr;
}
//----------------------------------------------------------------------
Response BackendApp::merge_responses(const Responses &responses)
{
  if (responses.size() == 0) {
    return make_response(HTTPStatus::NO_CONTENT);
  }
  else if (responses.size() == 1) {
    return responses.front();
  }
  else {
    auto to_json = [](const Response &response) {
      nlohmann::json json;
      json["status"] = static_cast<int>(response.status);
      if (response.body) {
        json["body"] = *response.body;
      }
      for (const auto &header : response.headers) {
        json[header.name] = header.value;
      }
      return std::move(json);
    };
    const auto body = ranges::accumulate(
        responses, nlohmann::json{},
        [&to_json](nlohmann::json &j, const Response &response) {
          j.emplace_back(to_json(response));
          return j;
        });

    return make_response(HTTPStatus::MULTI_STATUS, body);
  }
}
//----------------------------------------------------------------------
template <typename Entity>
std::string BackendApp::get_resource_uri(const Entity &entity)
{
  const auto id = Meta::get_id_column_ptr<Entity>()(entity);
  auto uri = fmt::format("{}/{}/{}", REST_API_VERSION, Meta::EntityName<Entity>,
                         id.value);
  return uri;
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
template <typename T>
bool BackendApp::is_entity_exists(typename Meta::IdColumnType<T> entity_id)
{
  return dao_.exists<T>(entity_id);
}
//----------------------------------------------------------------------
template <typename T>
T BackendApp::get_entity(typename Meta::IdColumnType<T> entity_id)
{
  auto entity = dao_.get_by_id<T>(entity_id);
  return entity;
}
//----------------------------------------------------------------------
template <typename T>
T BackendApp::update_entity(typename Meta::IdColumnType<T> /* entity_id */,
                            const nlohmann::json & /* entity_json */)
{
  NOT_IMPLEMENTED_YET();
}
//----------------------------------------------------------------------
template <typename T>
Meta::IdColumnType<T> BackendApp::create_entity(const T & /* entity */)
{
  static_assert(assert_false<T>::value, "Not implemented");
}

//----------------------------------------------------------------------
template <typename Entity>
void BackendApp::update_subentities(Entity &entity,
                                    Meta::IdColumnType<Entity> id)
{
  auto set_ids = [&id](auto &subentities) {
    using Sphinx::Db::LinkMany;
    using Link = LinkMany<decltype(subentities)>;
    if (subentities) {
      auto id_member_ptr = Meta::get_remote_key_member_ptr<Link>();
      auto set_id = [&id_member_ptr, &id](auto &subentity) {
        id_member_ptr(subentity).value = id;
      };
      ranges::for_each(*subentities, set_id);
    }
  };
  for_each_subentity_link(entity, set_ids);
}

//----------------------------------------------------------------------
template <typename Entity, typename Func>
void BackendApp::for_each_subentity_link(Entity &entity, Func &&func)
{
  auto subentities_links = entity.get_many_links();
  Utils::for_each_in_tuple(subentities_links, [&func](auto &subentities_link) {
    func(subentities_link);
  });
}

//----------------------------------------------------------------------
template <typename Entity>
void BackendApp::create_subentities(Entity &entity)
{
  auto func = [this](auto &subentities) {
    if (subentities)
      this->create_entities(*subentities);
  };
  for_each_subentity_link(entity, func);
}

//----------------------------------------------------------------------
template <typename T>
Responses BackendApp::create_entities(std::vector<T> &entities)
{
  Responses responses =
      entities | ranges::view::transform([this](auto &entity) {
        try {
          auto entity_id = this->create_entity(entity);
          entity.id.value = entity_id;
          this->update_subentities(entity, entity_id);
          this->create_subentities(entity);
          return make_response(HTTPStatus::CREATED,
                               Location{get_resource_uri(entity)});
        }
        catch (const std::runtime_error &ex) {
          auto message =
              fmt::format("Could not create entity. Message: {}", ex.what());
          logger()->error(message);
          return make_response(HTTPStatus::BAD_REQUEST, {{"message", message}});
        }
      });
  return responses;
}

//----------------------------------------------------------------------
template <typename T>
Responses BackendApp::create_entities(const nlohmann::json &data)
{
  auto entities = deserialize_entities<T>(data, true);
  const auto responses = create_entities(entities);
  return responses;
}

//----------------------------------------------------------------------
template <typename T>
Responses BackendApp::create_entities(const std::string &data)
{
  try {
    return create_entities<T>(nlohmann::json::parse(data));
  }
  catch (const std::invalid_argument &ex) {
    auto message =
        fmt::format("Could not parse the json. Message: {}", ex.what());
    logger()->error(message);
    return {make_response(HTTPStatus::BAD_REQUEST, {{"message", message}})};
  }
}

//----------------------------------------------------------------------
template <typename T>
void BackendApp::deserialize_subentities(const nlohmann::json &data, T &entity)
{
  auto links = entity.get_many_links();

  auto func = [this, &data, &entity](auto &link) {
    using Sphinx::Db::LinkMany;
    constexpr auto field_name = LinkMany<decltype(link)>::name;

    if (data.count(field_name)) {
      using RemoteEntity = typename LinkMany<decltype(link)>::remote_entity;
      link = this->deserialize_entities<RemoteEntity>(data[field_name]);
    }
    else {
      link = std::nullopt;
    }
  };
  Sphinx::Utils::for_each_in_tuple(links, func);
}

//----------------------------------------------------------------------
template <typename T>
std::vector<T> BackendApp::deserialize_entities(const nlohmann::json &data,
                                                const bool include_subentities)
{
  // TODO(przemkovv): clean up here a bit
  std::vector<T> entities;
  auto deserialize = [this, &include_subentities](const auto &entity_data) {
    using Sphinx::Json::from_json;
    if (include_subentities) {
      auto entity = from_json<T>(entity_data);
      deserialize_subentities(entity_data, entity);
      return entity;
    }
    else {
      return from_json<T>(entity_data);
    }
  };
  if (data.is_array() && data.size()) {
    entities.reserve(data.size());
    entities = data | ranges::view::transform(deserialize);
  }
  else if (data.is_object() && !data.empty()) {
    entities.emplace_back(deserialize(data));
  }
  return entities;
}

//----------------------------------------------------------------------
template <typename T>
std::vector<T> BackendApp::deserialize_entities(const std::string &data,
                                                const bool include_subentities)
{
  return deserialize_entities<T>(nlohmann::json::parse(data),
                                 include_subentities);
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
      auto r = create_entities(entities);
      return response(r);
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
          auto r = create_entities<Model::Module>(req.body);
          return response(r);
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
  try {
    logger()->debug("Configuration file: {}", config().dump(dump_indent_));

    add_users_routes();
    add_courses_routes();
    add_modules_routes();
    add_test_routes();

    app_.port(REST_API_PORT).run();

    return static_cast<int>(ExitCode::OK);
  }
  catch (const std::exception &ex) {
    logger()->error("Unhandled exception: {}.", ex.what());
    logger()->error("Quiting because of error.");
    return -1;
  }
}

} // namespace Sphinx::Backend
