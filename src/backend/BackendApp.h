
#pragma once

#include "Application.h"

#include "db/db.h"
#include <crow.h>

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

  const std::uint16_t SPHINX_BACKEND_PORT = 9998;
  const std::string API_VERSION = "/v1";

private:
  template <typename... Methods>
  auto &add_route(const std::string &path, const Methods... methods)
  {
    return app_.route_dynamic(API_VERSION + path).methods(methods...);
  }

  std::string get_users();
};

} // namespace Sphinx::Backend
