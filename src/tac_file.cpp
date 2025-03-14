#include "tac_file.hpp"

#include <cctype>
#include <fstream>
#include <indicators/progress_bar.hpp>
#include <iostream>
#include <memory>
#include <regex>
#include <string>

#include "utils.hpp"

static std::regex double_extension(R"(\.[\w]+\.[\w]+$)");

std::filesystem::path* TacFile::PrepareACMIFile(
    const std::filesystem::path& filepath) {
  if (!std::filesystem::exists(filepath)) {
    std::cerr << "File does not exist\n";
    return nullptr;
  }

  std::smatch match;
  std::string filepath_str = filepath.generic_string();
  if (std::regex_search(filepath_str, match, double_extension)) {
    if (match[0].str() == ".zip.acmi") {
      // Need to unzip it
      std::filesystem::path base_path =
          std::filesystem::temp_directory_path() / "TacView_Analyser";
      if (unzip_file(filepath, base_path) != 0) {
        std::cerr << "Failed to unzip file\n";
        return nullptr;
      }
      std::string new_filename = filepath.filename().generic_string();
      new_filename.resize(new_filename.size() - 9);
      new_filename += ".txt.acmi";
      return new std::filesystem::path(base_path / new_filename);
    } else if (match[0].str() == ".txt.acmi") {
      return new std::filesystem::path(filepath);
    } else {
      std::cerr << "File extension was not recognised" << std::endl;
      return nullptr;
    }
  } else {
    std::cerr << "File extension was not recognised" << std::endl;
    return nullptr;
  }

  return nullptr;
}

void TacFile::PrintPlayers(std::ostream& strm) {
  std::vector<const std::string*> players;
  uint32_t n = static_cast<uint32_t>(pilot_runs.size());
  players.reserve(n);
  for (const auto& [pilot, planes] : pilot_runs) {
    players.push_back(&pilot);
  }
  sort(players.begin(), players.end(),
       [](const std::string* a, const std::string* b) {
         return std::lexicographical_compare(
             a->begin(), a->end(), b->begin(), b->end(), [](char ac, char bc) {
               return std::tolower(ac) < std::tolower(bc);
             });
       });
  auto it = std::max_element(players.begin(), players.end(),
                             [](const std::string* a, const std::string* b) {
                               return a->size() < b->size();
                             });
  uint32_t maxSize = static_cast<uint32_t>((*it)->size());
  uint32_t columns = 5;
  uint32_t columnWidth = maxSize + 2;
  uint32_t rows = (n + columns - 1) / columns;
  for (uint32_t i = 0; i < rows; i++) {
    for (uint32_t j = 0; j < columns; j++) {
      uint32_t idx = i + j * rows;
      if (idx < n) {
        strm << std::left << std::setw(columnWidth) << *players[idx];
      }
    }
    strm << "\n";
  }
}

void TacFile::PrintAllPlayerRuns(std::ostream& strm) {
  for (auto it = pilot_runs.begin(); it != pilot_runs.end(); ++it) {
    for (const auto& plane : it->second) {
      plane->Print(strm, true, true, true, true, true);
      strm << "\n";
      for (const auto& mis : plane->getMissiles()) {
        strm << "\t";
        mis->Print(strm, true, true, false, false, false);
        strm << "\n";
      }
    }
  }
}

void TacFile::PrintPlayerRuns(std::ostream& strm, const std::string& username) {
  auto it = pilot_runs.find(username);
  if (it != pilot_runs.end()) {
    for (const auto& plane : it->second) {
      plane->Print(strm, true, true, true, true, true);
      strm << "\n";
      for (const auto& mis : plane->getMissiles()) {
        strm << "\t";
        mis->Print(strm, true, true, false, false, false);
        strm << "\n";
      }
    }
  } else {
    strm << "No data for " << username << "\n";
  }
}

void TacFile::LoadFile(const std::filesystem::path& filepath) {
  LoadFileWithChunks(filepath);
}
