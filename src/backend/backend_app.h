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
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/transform.hpp>

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
  crow::response response(int code);
  crow::response response(int code, const Header &header);
  crow::response response(int code, const std::string &body);
  crow::response response(int code, const nlohmann::json &body);
  template <typename T>
  crow::response response(int code, const T &data);
  crow::response response(const Response &r);
  crow::response response(const Responses &r);
  Response merge_responses(const Responses &responses);

  //----------------------------------------------------------------------
  template <typename Entity>
  std::string get_resource_uri(const Entity &entity);

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
  void add_maintenance_routes();

  //----------------------------------------------------------------------
  Model::Users get_users();
  Model::Courses get_courses();
  Model::Modules get_modules();
  Model::Modules get_modules(Meta::IdColumnType<Model::Course> course_id);

  //----------------------------------------------------------------------
  template <typename T>
  bool is_entity_exists(typename Meta::IdColumnType<T> entity_id);
  //----------------------------------------------------------------------
  template <typename T>
  T get_entity(typename Meta::IdColumnType<T> entity_id);
  //----------------------------------------------------------------------
  template <typename T>
  T update_entity(typename Meta::IdColumnType<T> /* entity_id */,
                  const nlohmann::json & /* entity_json */);
  //----------------------------------------------------------------------
  template <typename T>
  Meta::IdColumnType<T> create_entity(const T & /* entity */);

  //----------------------------------------------------------------------
  template <typename Entity, typename Func>
  void for_each_subentity_link(Entity &entity, Func &&func);

  //----------------------------------------------------------------------
  template <typename Entity>
  void update_subentities(Entity &entity, Meta::IdColumnType<Entity> id);

  template <typename Entity>
  void create_subentities(Entity &entity);

  //----------------------------------------------------------------------
  template <typename T>
  Responses create_entities(std::vector<T> &entities);

  template <typename T>
  Responses create_entities(const nlohmann::json &data);

  template <typename T>
  Responses create_entities(const std::string &data);

  //----------------------------------------------------------------------
  template <typename T>
  void deserialize_subentities(const nlohmann::json &data, T &entity);

  template <typename T>
  std::vector<T> deserialize_entities(const nlohmann::json &data,
                                      const bool include_subentities = false);

  template <typename T>
  std::vector<T> deserialize_entities(const std::string &data,
                                      const bool include_subentities = false);

  //----------------------------------------------------------------------
  Model::Users find_users(std::string name);
};
} // namespace Sphinx::Backend
