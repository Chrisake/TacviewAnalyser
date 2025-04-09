#include <Windows.h>

#include <argparse/argparse.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "global.hpp"
#include "tac_file.hpp"


std::filesystem::path exe_path;
std::filesystem::path exe_dir;

int main(int argc, char** argv) {
  argparse::ArgumentParser program("Tacview Analyser");
  program.add_argument("input-file")
    .required()
    .help("TacView file to analyse");

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;

    system("pause");
    std::exit(1);
  }

  exe_path = std::filesystem::path(argv[0]);
  exe_dir = exe_path.parent_path();

  /*CacheObject cache;
  cache.LoadCache();*/

  std::string              filename = program.get<std::string>("input-file");
  std::unique_ptr<TacFile> tac_file = std::make_unique<TacFile>();
  if (!tac_file->LoadFile(filename)) {
    std::cerr << "Failed to parse file\n";
    system("pause");
    return 1;
  }

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
    } catch (const std::exception& e) {
      std::cerr << e.what() << "\n";
    }
  }

  system("pause");
  return 0;
}