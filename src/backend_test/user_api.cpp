
#include "shared_lib/catch.hpp"
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <restclient-cpp/connection.h>
#include <restclient-cpp/restclient.h>
#include <string>

// TODO(przemkovv): add some configuration file
const std::string api_url = "http://localhost:9998";
const std::string samples_dir = "../../../docs/rest_api/samples/";

//----------------------------------------------------------------------
auto connect()
{
  return std::make_unique<RestClient::Connection>(api_url);
}

//----------------------------------------------------------------------
auto to_json(const std::string &body)
{
  return nlohmann::json::parse(body);
}

//----------------------------------------------------------------------
void cleanup_database()
{
  auto conn = connect();
  conn->post("/v1/maintenance/database_cleanup", "");
}
//----------------------------------------------------------------------

std::string read_data(const std::string &name)
{
  auto filename = samples_dir + name + ".json";
  std::ifstream file{filename};
  if (file)
    return std::string{(std::istreambuf_iterator<char>(file)), {}};
  else
    throw std::runtime_error("Could open file " + filename);
}

template <typename Conn>
void there_is_no_users(Conn &conn)
{
  auto response = conn->get("/v1/users");
  REQUIRE(response.code == 200);
  REQUIRE(to_json(response.body).size() == 0);
  REQUIRE(response.headers["Content-Type"] == "application/json");
}
//----------------------------------------------------------------------

SCENARIO("User can be added and deleted", "[users]")
{

  GIVEN("An empty database")
  {
    cleanup_database();

    auto conn = connect();
    auto response = conn->get("/v1/users");
    REQUIRE(response.code == 200);
    REQUIRE(to_json(response.body).size() == 0);
    REQUIRE(response.headers["Content-Type"] == "application/json");

    WHEN("the user is added")
    {
      response = conn->post("/v1/users", read_data("create_user"));
      REQUIRE(response.code == 201);

      THEN("there is one user")
      {
        response = conn->get("/v1/users");
        REQUIRE(response.code == 200);
        REQUIRE(to_json(response.body).size() == 1);
        REQUIRE(response.headers["Content-Type"] == "application/json");
      }
    }

    WHEN("the an invliad user object is passed")
    {
      response =
          conn->post("/v1/users", read_data("create_user_invalid_field"));
      REQUIRE(response.code == 400);

      THEN("there is no users") { there_is_no_users(conn); }
    }
    WHEN("the an empty user object is passed")
    {
      response = conn->post("/v1/users", read_data("create_user_empty"));
      REQUIRE(response.code == 204);

      THEN("there is no users") { there_is_no_users(conn); }
    }
  }
}
