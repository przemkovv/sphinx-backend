#pragma once

#include <Logger.h>

#include <boost/program_options.hpp>
#include <memory>
#include <string>

#include <nlohmann/json.hpp>

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
  Logger logger_;

  const std::string application_name_;
  const std::vector<std::string> args_;
  po::options_description options_description_cli_;
  po::variables_map config_cli_;

  json config_;

public:
  Application(const std::string &application_name,
              const std::vector<std::string> &args);
  virtual int run() = 0;

  virtual ~Application() {}
};
}
