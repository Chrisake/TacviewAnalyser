#pragma once

#include "data_service.hpp"
#include "server_configuration.hpp"

#include <string>
#include <filesystem>
#include <memory>

extern std::filesystem::path exe_path;
extern std::filesystem::path exe_dir;

extern std::unique_ptr<DataService> data_service;
extern std::unique_ptr<ServerConfiguration> server_config;
