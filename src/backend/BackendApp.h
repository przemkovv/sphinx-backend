
#pragma once

#include "Application.h"

#include "db/db.h"
#include <crow.h>

using json_pointer = nlohmann::json::json_pointer;
using std::literals::operator""s;

namespace Sphinx::Backend {

class BackendApp : public Application {

public:
  using Application::Application;

  BackendApp(const std::string &application_name,
             const std::vector<std::string> &args);

  int run() override;

protected:
  Db::connection_config prepare_db_config();

  Db::connection_config db_config_;
  Db::connection db_;

  crow::SimpleApp app_;

  const json_pointer REST_API_PORT_PATH = "/server/port"_json_pointer;
  const json_pointer REST_API_VERSION_PATH = "/server/version"_json_pointer;
  const std::uint16_t REST_API_PORT;
  const std::string REST_API_VERSION;

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
};

} // namespace Sphinx::Backend
