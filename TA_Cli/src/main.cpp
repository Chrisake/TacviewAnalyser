#include <Windows.h>

#include <argparse/argparse.hpp>
#include <indicators/progress_bar.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "global.hpp"
#include "logging.hpp"
#include "tac_file.hpp"

std::filesystem::path exe_path;
std::filesystem::path exe_dir;

static std::shared_ptr<spdlog::logger> logger = nullptr;

static void progressThread(size_t &cur_prog, size_t &max_prog) {
  using namespace indicators;
  ProgressBar progress{
      option::BarWidth{80},
      option::Start{"["},
      option::End{"]"},
      option::Fill{"="},
      option::Lead{">"},
      option::ForegroundColor{Color::white},
      option::ShowElapsedTime{true},
      option::ShowRemainingTime{true},
      option::ShowPercentage{true},
      option::FontStyles{std::vector<FontStyle>{FontStyle::bold}},
      option::MaxProgress(max_prog),
  };
  while (true) {
    progress.set_option(option::MaxProgress(max_prog));
    progress.set_progress(static_cast<size_t>(cur_prog));
    if (progress.is_completed()) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::cout << "\n";
}

int main(int argc, char **argv) {
  logger = initialize_logger("ta_cli", spdlog::level::trace);
  argparse::ArgumentParser program("Tacview Analyser");
  program.add_argument("input-file").required().help("TacView file to analyse");

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    logger->critical("Failed to parse command line arguments. Error: %s", err.what());
    system("pause");
    std::exit(1);
  }

  exe_path = std::filesystem::path(argv[0]);
  exe_dir = exe_path.parent_path();

  /*CacheObject cache;
  cache.LoadCache();*/

  std::string              filename = program.get<std::string>("input-file");
  std::unique_ptr<TacFile> tac_file = std::make_unique<TacFile>();
  size_t                   cur_prog = 0;
  size_t                   max_prog = 100;

  std::thread progress_thread(progressThread, std::ref(cur_prog), std::ref(max_prog));
  if (!tac_file->LoadFile(filename, cur_prog, max_prog)) {
    logger->critical("Failed to parse file: %s.", filename);
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
    } catch (const std::exception &e) {
      logger->critical("Failed to print player runs for %s. Error: %s", username, e.what());
    }
  }
  logger->flush();
  logger.reset();
  system("pause");
  return 0;
}