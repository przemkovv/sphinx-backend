
#include "db_utils.h"

namespace Sphinx::Db {
//----------------------------------------------------------------------
template <>
int64_t convert_to(const char *data)
{
  return std::stoll(data);
}

//----------------------------------------------------------------------
template <>
std::string convert_to(const char *data)
{
  return {data};
}

//----------------------------------------------------------------------
template <>
optional<std::string> to_optional_string(std::nullptr_t /* null */)
{
  return {};
}
} // namespace Sphinx::Db
