#pragma once

#include <filesystem>
#include <string>

class ServerConfiguration {
 public:
  struct DatabaseConfig {
    std::string host;
    uint16_t port;
    std::string username;
    std::string password;
    std::string database;
  };

  struct ServerConfig {
    std::string host;
    uint16_t port;
  };

  struct LogConfig {
    std::filesystem::path log_directory;
    int log_level;
  };

  ServerConfiguration() = default;
  bool LoadYamlConfig();
  bool LoadYamlConfig(const std::filesystem::path &path);
  DatabaseConfig database_config;
  ServerConfig web_api_config;
  LogConfig log_config;
};
