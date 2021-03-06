#include "application.h"
#include <boost/lexical_cast/bad_lexical_cast.hpp>  // for bad_lexical_cast
#include <boost/program_options/parsers.hpp>        // for command_line_parser
#include <boost/program_options/value_semantic.hpp> // for typed_value, value
#include <boost/type_index/type_index_facade.hpp>   // for operator==
#include <exception>                                // for exception
#include <fstream>                                  // for ifstream
#include <memory>                                   // for __shared_ptr_access

namespace Sphinx {

//----------------------------------------------------------------------
Application::Application(const std::string &application_name,
                         const std::vector<std::string> &args)
  : application_name_(application_name),
    args_(args),
    options_description_cli_(prepare_options_description_cli()),
    config_cli_(parse_command_line_options(args_)),
    config_(parse_config_file()),
    logger_(Sphinx::make_logger("Application"))
{

  auto log_level = config_cli_["log_level"].as<int>();
  configure_logger(static_cast<spdlog::level::level_enum>(log_level));
}

//----------------------------------------------------------------------
po::options_description Application::prepare_options_description_cli()
{
  po::options_description desc("Allowed options");
  /* clang-format off */
  desc.add_options()
    ("help,h", "Display help information")
    ("log_level", po::value<int>()->default_value(spdlog::level::info), "Logging level")
    ("config", po::value<std::string>()->required(), "Configuration file");
  /* clang-format on */
  return desc;
}

//----------------------------------------------------------------------
po::variables_map
Application::parse_command_line_options(const std::vector<std::string> &args)
{
  po::variables_map vm;
  po::store(
      po::command_line_parser(args).options(options_description_cli_).run(),
      vm);
  po::notify(vm);
  return vm;
}

//----------------------------------------------------------------------
json Application::parse_config_file()
{
  json config;
  if (config_cli_.count("config")) {
    auto config_file = config_cli_["config"].as<std::string>();
    std::ifstream file(config_file);
    config << file;
  }
  return config;
}

//----------------------------------------------------------------------
void Application::configure_logger(spdlog::level::level_enum log_level)
{
  spdlog::set_level(log_level);
  logger_ = make_logger(application_name_);

  auto logger_levels = config_["loggers"];
  for (auto it = logger_levels.begin(); it != logger_levels.end(); ++it) {
    logger()->debug("Logger {} has log level {}.", it.key(),
                    it.value().get<int>());
    make_logger(it.key(),
                static_cast<spdlog::level::level_enum>(it.value().get<int>()));
  }
}

} // namespace Sphinx
