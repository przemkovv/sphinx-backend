#pragma once

#include "application.h"

#include "dao.h"
#include "db/model_meta.h"
#include "http_status_code.h"
#include "model/model_relations.h"
#include "rest_helper.h"
#include "shared_lib/for_each_in_tuple.h"
#include "json/json_serializer.h"
#include <crow.h>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>

using json_pointer = nlohmann::json::json_pointer;
using namespace std::literals::string_literals;

namespace Meta = Sphinx::Db::Meta;

namespace Sphinx::Backend {

class BackendApp : public Application {

public:
  using Application::Application;

  BackendApp(const std::string &application_name,
             const std::vector<std::string> &args);

  int run() override;

protected:
  Sphinx::Db::connection_config prepare_db_config();

  DAO dao_;

  crow::SimpleApp app_;

  const json_pointer REST_API_PORT_PATH = "/server/port"_json_pointer;
  const json_pointer REST_API_VERSION_PATH = "/server/version"_json_pointer;
  const json_pointer DUMP_LEVEL_PATH = "/debug/dump_level"_json_pointer;
  const std::uint16_t REST_API_PORT;
  const std::string REST_API_VERSION;

  const int dump_indent_;

private:
  //----------------------------------------------------------------------
  auto response(int code) { return crow::response(code); }

  //----------------------------------------------------------------------
  auto response(int code, const Header &header)
  {
    auto r = crow::response(code);
    r.set_header(header.name, header.value);
    return r;
  }

  //----------------------------------------------------------------------
  auto response(int code, const std::string &body)
  {
    return crow::response(code, body);
  }

  //----------------------------------------------------------------------
  auto response(int code, const nlohmann::json &body)
  {
    auto r = crow::response(code, body.dump(dump_indent_));
    r.set_header("Content-Type", "application/json");
    return r;
  }

  //----------------------------------------------------------------------
  template <typename T>
  auto response(int code, const T &data)
  {
    const auto body = Json::to_json(data);
    return response(code, body);
  }

  //----------------------------------------------------------------------
  auto response(const Response &r)
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
  template <typename Entity>
  auto get_resource_uri(const Entity &entity)
  {
    const auto id = Meta::get_id_column_ptr<Entity>()(entity);
    auto uri = fmt::format("{}/{}/{}", REST_API_VERSION,
                           Meta::EntityName<Entity>, id.value);
    return uri;
  }

  //----------------------------------------------------------------------
  template <typename... Methods>
  auto &add_route(const std::string &path, const Methods... methods)
  {
    const auto route = REST_API_VERSION + path;
    logger()->debug("Adding route {} (methods:{})", route,
                    ((" "s + crow::method_name(methods)) + ...));
    return app_.route_dynamic(REST_API_VERSION + path).methods(methods...);
  }

  //----------------------------------------------------------------------
  void add_users_routes();
  void add_courses_routes();
  void add_modules_routes();
  void add_test_routes();

  //----------------------------------------------------------------------
  Model::Users get_users();
  Model::Courses get_courses();
  Model::Modules get_modules();
  Model::Modules get_modules(Meta::IdColumnType<Model::Course> course_id);

  //----------------------------------------------------------------------
  template <typename T>
  bool is_entity_exists(typename Meta::IdColumnType<T> entity_id)
  {
    return dao_.exists<T>(entity_id);
  }
  //----------------------------------------------------------------------
  template <typename T>
  T get_entity(typename Meta::IdColumnType<T> entity_id)
  {
    auto entity = dao_.get_by_id<T>(entity_id);
    return entity;
  }
  //----------------------------------------------------------------------
  template <typename T>
  T update_entity(typename Meta::IdColumnType<T> /* entity_id */,
                  const nlohmann::json & /* entity_json */)
  {
    NOT_IMPLEMENTED_YET();
  }
  //----------------------------------------------------------------------
  template <typename T>
  Meta::IdColumnType<T> create_entity(const T & /* entity */)
  {
    static_assert(assert_false<T>::value, "Not implemented");
  }

  //----------------------------------------------------------------------

  template <typename Entity>
  void update_subentities(Entity &entity, Meta::IdColumnType<Entity> id)
  {
    auto set_id = [&id](auto &subentities) {
      using Sphinx::Db::LinkMany;
      using Link = LinkMany<decltype(subentities)>;
      if (subentities) {
        auto member_ptr = Meta::get_remote_key_member_ptr<Link>();
        for (auto &subentity : *subentities) {
          member_ptr(subentity).value = id;
        }
      }
    };
    for_each_subentity_link(entity, set_id);
  }

  //----------------------------------------------------------------------
  template <typename Entity, typename Func>
  void for_each_subentity_link(Entity &entity, Func &&func)
  {
    auto subentities_links = entity.get_many_links();
    Utils::for_each_in_tuple(
        subentities_links,
        [&func](auto &subentities_link) { func(subentities_link); });
  }

  //----------------------------------------------------------------------
  template <typename Entity>
  void create_subentities(Entity &entity)
  {
    auto func = [this](auto &subentities) {
      if (subentities)
        this->create_entities(*subentities);
    };
    for_each_subentity_link(entity, func);
  }

  //----------------------------------------------------------------------
  template <typename T>
  auto create_entities(std::vector<T> &entities)
  {
    std::vector<Response> responses;
    std::transform(
        entities.begin(), entities.end(), std::back_inserter(responses),
        [this](auto &entity) -> Response {
          try {
            auto entity_id = this->create_entity(entity);
            entity.id.value = entity_id;
            this->update_subentities(entity, entity_id);
            this->create_subentities(entity);
            Response r{HTTPStatus::CREATED};
            r.headers.emplace_back(Location{get_resource_uri(entity)});
            return r;
          }
          catch (const std::runtime_error &ex) {
            auto message =
                fmt::format("Could not create entity. Message: {}", ex.what());
            logger()->error(message);
            Response r{HTTPStatus::BAD_REQUEST};
            r.body = nlohmann::json{{"message", message}};
            return r;
          }
        });
    return responses;
  }

  //----------------------------------------------------------------------
  template <typename T>
  Response create_entities(const nlohmann::json &data)
  {
    auto entities = deserialize_entities<T>(data, true);
    const auto responses = create_entities(entities);
    if (responses.size() == 0) {
      return {HTTPStatus::NO_CONTENT};
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
      const auto body = std::accumulate(
          responses.begin(), responses.end(), nlohmann::json{},
          [&to_json](nlohmann::json& j, const Response &response) {
            j.emplace_back(to_json(response));
            return j;
          });

      return {HTTPStatus::MULTI_STATUS, body};
    }
  }

  //----------------------------------------------------------------------
  template <typename T>
  Response create_entities(const std::string &data)
  {
    try {
      return create_entities<T>(nlohmann::json::parse(data));
    }
    catch (const std::invalid_argument &ex) {
      auto message =
          fmt::format("Could not parse the json. Message: {}", ex.what());
      logger()->error(message);
      return {HTTPStatus::BAD_REQUEST, nlohmann::json{{"message", message}}};
    }
  }

  //----------------------------------------------------------------------
  template <typename T>
  void deserialize_subentities(const nlohmann::json &data, T &entity)
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
  std::vector<T> deserialize_entities(const nlohmann::json &data,
                                      const bool include_subentities = false)
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
      std::transform(data.begin(), data.end(), std::back_inserter(entities),
                     deserialize);
    }
    else if (data.is_object() && !data.empty()) {
      entities.emplace_back(deserialize(data));
    }
    return entities;
  }

  //----------------------------------------------------------------------
  template <typename T>
  std::vector<T> deserialize_entities(const std::string &data,
                                      const bool include_subentities = false)
  {
    return deserialize_entities<T>(nlohmann::json::parse(data),
                                   include_subentities);
  }

  Model::Users find_users(std::string name);
};
} // namespace Sphinx::Backend
