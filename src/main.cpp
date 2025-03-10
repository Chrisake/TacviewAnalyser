#include <Windows.h>

#include <boost/program_options.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "cache.hpp"
#include "global.hpp"
#include "tac_file.hpp"

namespace po = boost::program_options;

std::filesystem::path exe_path;
std::filesystem::path exe_dir;

int main(int argc, char** argv) {
  exe_path = std::filesystem::path(argv[0]);
  exe_dir = exe_path.parent_path();

  /*CacheObject cache;
  cache.LoadCache();*/

  po::options_description opts_desc("Allowed options");
  opts_desc.add_options()("help", "")(
      "input-file", po::value<std::string>(), "TacView file to analyse");

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
  } catch (const std::exception& e) {
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
      } catch (const std::exception& e) {
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