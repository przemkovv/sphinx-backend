
#include "db_utils.h"

namespace Sphinx::Db {
//----------------------------------------------------------------------
template <>
bool convert_to(const char *data)
{
  return data[0] == 't' || data[0] == '1';
}

//----------------------------------------------------------------------
template <>
int64_t convert_to(const char *data)
{
  return std::stoll(data);
}

//----------------------------------------------------------------------
template <>
uint64_t convert_to(const char *data)
{
  return std::stoull(data);
}

//----------------------------------------------------------------------
template <>
std::string convert_to(const char *data)
{
  return {data};
}

//----------------------------------------------------------------------
template <>
std::optional<std::string> to_optional_string(std::nullptr_t /* null */)
{
  return {};
}
} // namespace Sphinx::Db
