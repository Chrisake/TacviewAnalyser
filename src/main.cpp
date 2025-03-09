#include "global.hpp"
#include "cache.hpp"
#include "tac_file.hpp"

#include <boost/program_options.hpp>

#include <Windows.h>

#include <iostream>
#include <string>
#include <memory>

namespace po = boost::program_options;

std::filesystem::path exe_path;
std::filesystem::path exe_dir;

int main(int argc, char** argv) {

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

	exe_path = std::filesystem::path(argv[0]);
	exe_dir = exe_path.parent_path();

	CacheObject cache;
	cache.LoadCache();

    po::options_description opts_desc("Allowed options");
    opts_desc.add_options()
        ("help", "produce help message")
        ("input-file", po::value<std::string>(), "TacView file to analyse")
        ("username", po::value<std::string>()->default_value(cache.username), "Pilot's username");

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
    }
	catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
		return 1;
	}
    if (vm.count("help")) {
        std::cout << opts_desc << "\n";
        return 0;
    }
    
    cache.username = vm["username"].as<std::string>();
    while (cache.username.empty()) {
		std::cerr << "Username: ";
		getline(std::cin, cache.username);
    }
	cache.SaveCache();

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
	}
	else {
		std::cerr << "No input file specified\n";
		return 1;
	}

    return 0;
}