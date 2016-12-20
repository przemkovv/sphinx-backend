#pragma once

#include "http_status_code.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

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
struct Response {
  HTTPStatus status;
  std::optional<nlohmann::json> body;
  std::vector<Header> headers;

  Response(HTTPStatus status_,
           std::optional<nlohmann::json> body_,
           std::vector<Header> headers_)
    : status(status_), body(std::move(body_)), headers(std::move(headers_))
  {
  }
  Response(HTTPStatus status_) : Response(status_, {}, {}) {}
  Response(HTTPStatus status_, nlohmann::json body_)
    : Response(status_, std::make_optional(std::move(body_)), {})
  {
  }
  Response(HTTPStatus status_, std::vector<Header> headers_)
    : Response(status_, {}, headers_)
  {
  }
};

} // namespace Sphinx::Backend
