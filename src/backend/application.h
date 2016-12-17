#pragma once

#include "shared_lib/logger.h"                           // for Logger
#include <boost/program_options/errors.hpp>              // for program_opt...
#include <boost/program_options/options_description.hpp> // for options_des...
#include <boost/program_options/variables_map.hpp>       // for variables_map
#include <nlohmann/json.hpp>                             // for json
#include <spdlog/spdlog.h>                               // for level_enum
#include <string>                                        // for string
#include <vector>                                        // for vector

using json = nlohmann::json;
using json_pointer = nlohmann::json::json_pointer;

namespace Sphinx {

namespace po = boost::program_options;

class Application {
private:
  void configure_logger(spdlog::level::level_enum level);

  po::options_description prepare_options_description_cli();
  po::variables_map
  parse_command_line_options(const std::vector<std::string> &arguments);
  json parse_config_file();

protected:
  template <typename T>
  auto config_get(const json_pointer &path)
  {
    return config_[path].get<T>();
  }
  auto &logger() { return logger_; }
  auto &config() { return config_; }
  auto &config_cli() { return config_cli_; }
  auto &options_description_cli() { return options_description_cli_; }

private:
  const std::string application_name_;
  const std::vector<std::string> args_;
  po::options_description options_description_cli_;
  po::variables_map config_cli_;

  json config_;

  Logger logger_;

public:
  Application(const std::string &application_name,
              const std::vector<std::string> &args);
  virtual int run() = 0;

  virtual ~Application() {}
};
}
