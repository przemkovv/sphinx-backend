
#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <memory>
#include <string>

namespace Sphinx {

typedef std::shared_ptr<spdlog::logger> Logger;

inline auto make_logger(const std::string &name)
{
  try {
    return spdlog::stdout_logger_st(name, true);
  }
  catch (spdlog::spdlog_ex &) {
    return spdlog::get(name);
  }
}

inline auto make_logger(const std::string &name,
                        const spdlog::level::level_enum &level)
{
  try {
    auto logger = spdlog::stdout_logger_st(name, true);
    logger->set_level(level);
    return logger;
  }
  catch (spdlog::spdlog_ex &) {
    auto logger =  spdlog::get(name);
    logger->set_level(level);
    return logger;
  }
}

} // namespace Sphinx
