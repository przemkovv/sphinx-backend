
#include "shared_lib/catch.hpp"
#include <memory>
#include <nlohmann/json.hpp>
#include <restclient-cpp/connection.h>
#include <restclient-cpp/restclient.h>
#include <string>

std::string api_url = "http://localhost:9998";

auto connect()
{
  return std::make_unique<RestClient::Connection>(api_url);
}

auto to_json(const std::string &body)
{
  return nlohmann::json::parse(body);
}
void cleanup_database(){
  auto conn = connect();
  conn->post("/v1/maintenance/database", "");
}


SCENARIO("User can be added and deleted", "[users]")
{

  GIVEN("An empty database")
  {
    auto conn = connect();

    auto response = conn->get("/v1/users");
    REQUIRE(response.code == 200);
    REQUIRE(to_json(response.body).size() == 0);
    REQUIRE(response.headers["Content-Type"] == "application/json");

    // GET /v1/users is empty

    WHEN("the user is added")
    {
      // POST /v1/users

      THEN("there is one user")
      {
        /// GET /v1/users returns one user
      }
    }
  }
}
