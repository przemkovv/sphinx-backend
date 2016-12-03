
#pragma once

#include "Application.h"

#include "db.h"
#include "db/dao.h"
#include "model_meta.h"
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

  Sphinx::Backend::Db::DAO dao_;

  crow::SimpleApp app_;

  const json_pointer REST_API_PORT_PATH = "/server/port"_json_pointer;
  const json_pointer REST_API_VERSION_PATH = "/server/version"_json_pointer;
  const json_pointer DUMP_LEVEL_PATH = "/debug/dump_level"_json_pointer;
  const std::uint16_t REST_API_PORT;
  const std::string REST_API_VERSION;

  const int dump_indent_;

private:
  template <typename... Methods>
  auto &add_route(const std::string &path, const Methods... methods)
  {
    const auto route = REST_API_VERSION + path;
    logger()->debug("Adding route {} (methods:{})", route,
                    ((" "s + crow::method_name(methods)) + ...));
    return app_.route_dynamic(REST_API_VERSION + path).methods(methods...);
  }
  void add_users_routes();
  void add_courses_routes();
  void add_modules_routes();
  void add_test_routes();

  std::string get_users();
  std::string get_courses();
  std::string get_modules();
  std::string get_modules(typename Meta::IdColumn_t<Model::Course> course_id);

  template <typename T>
  bool is_entity_exists(typename Sphinx::Db::Meta::IdColumn<T>::type entity_id)
  {
    return dao_.exists<T>(entity_id);
  }
  template <typename T>
  T get_entity(typename Sphinx::Db::Meta::IdColumn<T>::type entity_id)
  {
    auto entity = dao_.get_by_id<T>(entity_id);
    return entity;
  }
  template <typename T>
  T update_entity(typename Sphinx::Db::Meta::IdColumn<T>::type entity_id,
                  const nlohmann::json &entity_json)
  {
    NOT_IMPLEMENTED_YET();
  }
  template <typename T>
  void create_entity(const nlohmann::json &entity_json)
  {
    static_assert(assert_false<T>::value, "Not implemented");
  }
  template <typename T>
  void create_entities(const nlohmann::json &data)
  {
    if (data.is_array()) {
      for (const auto &entity_json : data) {
        create_entity<T>(entity_json);
      }
    }
    else {
      create_entity<T>(data);
    }
  }

  template <typename T>
  void create_entities(const std::string &data)
  {
    return create_entities<T>(nlohmann::json::parse(data));
  }

  std::string find_users(std::string name);
};
} // namespace Sphinx::Backend
