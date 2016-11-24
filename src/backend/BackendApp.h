
#pragma once

#include "Application.h"

#include "db.h"
#include "db/dao.h"
#include <crow.h>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>

using json_pointer = nlohmann::json::json_pointer;
using namespace std::literals::string_literals;

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

  std::string get_users();
  std::string get_courses();
  std::string get_modules();

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
};
} // namespace Sphinx::Backend
