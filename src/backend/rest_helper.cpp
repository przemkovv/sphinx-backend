
#include "rest_helper.h"

namespace Sphinx::Backend {

//----------------------------------------------------------------------
Response::Response(HTTPStatus status_,
                   std::optional<nlohmann::json> body_,
                   Headers headers_)
  : status(status_), body(std::move(body_)), headers(std::move(headers_))
{
}

//----------------------------------------------------------------------
Response::Response(HTTPStatus status_) : Response(status_, {}, {})
{
}

//----------------------------------------------------------------------
Response::Response(HTTPStatus status_, nlohmann::json body_)
  : Response(status_, std::make_optional(std::move(body_)), {})
{
}

//----------------------------------------------------------------------
Response make_response(HTTPStatus status)
{
  return {status};
}

//----------------------------------------------------------------------
Response make_response(HTTPStatus status, nlohmann::json json)
{
  return {status, std::move(json)};
}
} // namespace Sphinx::Backend
