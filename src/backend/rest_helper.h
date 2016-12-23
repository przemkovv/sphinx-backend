#pragma once

#include "http_status_code.h" // for HTTPStatus
#include <algorithm>          // for move
#include <nlohmann/json.hpp>  // for json
#include <optional>           // for optional
#include <string>             // for string
#include <type_traits>        // for remove_reference<>::type
#include <vector>             // for vector

namespace Sphinx::Backend {

//----------------------------------------------------------------------
struct Header {
  std::string name;
  std::string value;
};

//----------------------------------------------------------------------
struct Location : Header {
  Location(std::string uri) : Header{"Location", std::move(uri)} {}
};

//----------------------------------------------------------------------
using Headers = std::vector<Header>;

struct Response;
using Responses = std::vector<Response>;

//----------------------------------------------------------------------
struct Response {
  HTTPStatus status;
  std::optional<nlohmann::json> body;
  Headers headers;

  Response(HTTPStatus status_,
           std::optional<nlohmann::json> body_,
           Headers headers_);
  Response(HTTPStatus status_);
  Response(HTTPStatus status_, nlohmann::json body_);
};

//----------------------------------------------------------------------
Response make_response(HTTPStatus status);
Response make_response(HTTPStatus status, nlohmann::json json);

//----------------------------------------------------------------------
template <typename... Hs>
auto make_response(HTTPStatus status, nlohmann::json json, Hs &&... headers)
{
  return Response{status, {std::move(json)}, Headers{std::move(headers)...}};
}

//----------------------------------------------------------------------
template <typename... Hs>
auto make_response(HTTPStatus status, Hs &&... headers)
{
  return Response{status, {}, Headers{std::move(headers)...}};
}

} // namespace Sphinx::Backend