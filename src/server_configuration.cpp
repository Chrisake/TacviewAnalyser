#include "server_configuration.hpp"

#include "global.hpp"
#include "yaml-cpp/yaml.h"

std::unique_ptr<ServerConfiguration> server_config;

bool ServerConfiguration::LoadYamlConfig(const std::filesystem::path &path) {
  try {
    YAML::Node config = YAML::LoadFile(path.string());
    database_config.host = config["database"]["host"].as<std::string>();
    database_config.port = config["database"]["port"].as<uint16_t>();
    database_config.username = config["database"]["username"].as<std::string>();
    database_config.password = config["database"]["password"].as<std::string>();
    database_config.database = config["database"]["database"].as<std::string>();
    web_api_config.host = config["server"]["host"].as<std::string>();
    web_api_config.port = config["server"]["port"].as<uint16_t>();
    log_config.log_directory =
        exe_dir / config["logging"]["directory"].as<std::string>();
    log_config.log_level = config["logging"]["level"].as<int>();
  } catch (const YAML::Exception &e) {
    std::cerr << "Error loading configuration: " << e.what() << std::endl;
    return false;
  }
  return true;
}

bool ServerConfiguration::LoadYamlConfig() {
  return LoadYamlConfig(exe_dir / "config.yaml");
}