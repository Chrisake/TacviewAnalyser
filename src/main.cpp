#include <drogon/drogon.h>

#include <boost/program_options.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "cache.hpp"
#include "data_service.hpp"
#include "global.hpp"
#include "tac_file.hpp"

namespace po = boost::program_options;

std::filesystem::path exe_path;
std::filesystem::path exe_dir;

int main(int argc, char **argv) {
  exe_path = std::filesystem::path(argv[0]);
  exe_dir = exe_path.parent_path();

  server_config = std::make_unique<ServerConfiguration>();
  if (!server_config->LoadYamlConfig()) {
    std::cerr << "Error loading configuration. Exiting..." << std::endl;
    return 1;
  }

  data_service = std::make_unique<DataService>(
      server_config->database_config.database,
      server_config->database_config.username,
      server_config->database_config.password,
      server_config->database_config.host, server_config->database_config.port);
  if (!data_service->Test()) {
    std::cerr << "Database Connection is required. Exiting..." << std::endl;
    return 1;
  }
  data_service->Init(10);

  std::filesystem::create_directory(server_config->log_config.log_directory);
  std::filesystem::create_directory(server_config->log_config.log_directory /
                                    "web");
  std::filesystem::create_directory(server_config->log_config.log_directory /
                                    "server");
  drogon::app()
      .setLogPath((server_config->log_config.log_directory / "web").string())
      .setLogLevel(static_cast<trantor::Logger::LogLevel>(
          server_config->log_config.log_level))
      .addListener(server_config->web_api_config.host,
                   server_config->web_api_config.port)
      .setThreadNum(16)
      .run();

  while (true) {
    Sleep(1000);
  }

  /*CacheObject cache;
  cache.LoadCache();*/

  po::options_description opts_desc("Allowed options");
  opts_desc.add_options()("help", "")("input-file", po::value<std::string>(),
                                      "TacView file to analyse");

  po::positional_options_description pos_opts_desc;
  pos_opts_desc.add("input-file", 1);

  po::variables_map vm;
  try {
    auto parsed = po::command_line_parser(argc, argv)
                      .options(opts_desc)
                      .positional(pos_opts_desc)
                      .run();
    po::store(parsed, vm);
    po::notify(vm);
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
  if (vm.count("help")) {
    std::cout << opts_desc << "\n";
    system("pause");
    return 0;
  }

  if (vm.count("input-file")) {
    std::string filename = vm["input-file"].as<std::string>();
    std::unique_ptr<TacFile> tac_file = std::make_unique<TacFile>();
    tac_file->LoadFile(filename);
    tac_file->PrintPlayers(std::cout);
    while (true) {
      std::string username;
      std::cout << "Enter username to view: ";
      getline(std::cin, username);
      if (username.empty()) {
        break;
      }
      try {
        tac_file->PrintPlayerRuns(std::cout, username);
      } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
      }
    }
  } else {
    std::cerr << "No input file specified\n";
    std::cout << opts_desc << "\n";
    system("pause");
    return 1;
  }

  return 0;
}