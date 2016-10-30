
#include "db/db.h"
#include <Logger.h>
#include <crow.h>
#include <fmt/format.h>

#include <json_diag.hpp>

#include <sqlpp11/sqlite3/sqlite3.h>
#include <sqlpp11/sqlpp11.h>
#include <string>
#include <tuple>
#include <vector>

using std::literals::operator""s;

int main()
{

  const int SPHINX_BACKEND_PORT = 9998;
  const auto REST_API_VERSION = "/v1"s;

  sqlpp::sqlite3::connection_config config;
  config.path_to_database =
      "/home/przemkovv/projects/sphinx/backend/data/sphinx_backend.sqlite";
  config.flags = SQLITE_OPEN_READWRITE;
  config.debug = true;

  sqlpp::sqlite3::connection db(config);

  const auto users = Sphinx::Db::Users{};

  crow::SimpleApp app;
  auto logger = Sphinx::make_logger("Sphinx::Backend");

  app.route_dynamic(REST_API_VERSION + "/users")([&](const crow::request &req) {
    if (req.method == "GET"_method) {
      nlohmann::json response;
      for (const auto &row : db(select(users.username, users.email)
                                    .from(users)
                                    .unconditionally())) {
        response.push_back({{"username", row.username}, {"email", row.email}});
      }
      return response.dump(2);
    }
    return ""s;
  });

  app.port(SPHINX_BACKEND_PORT).run();

  return 0;
}
