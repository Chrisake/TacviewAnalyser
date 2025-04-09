#include "tac_file.hpp"

#include <cctype>
#include <fstream>
#include <indicators/progress_bar.hpp>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <thread>

#include "utils.hpp"

static std::regex double_extension(R"(\.[\w]+\.[\w]+$)");

std::filesystem::path *TacFile::PrepareACMIFile(const std::filesystem::path &filepath) {
  if (!std::filesystem::exists(filepath)) {
    std::cerr << "File does not exist\n";
    return nullptr;
  }

  std::smatch match;
  std::string filepath_str = filepath.generic_string();
  if (std::regex_search(filepath_str, match, double_extension)) {
    if (match[0].str() == ".zip.acmi") {
      // Need to unzip it
      std::filesystem::path base_path = std::filesystem::temp_directory_path() / "TacView_Analyser";
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

void TacFile::PrintPlayers(std::ostream &strm) {
  std::vector<const std::string *> players;
  uint32_t                         n = static_cast<uint32_t>(pilot_runs.size());
  players.reserve(n);
  for (const auto &[pilot, planes] : pilot_runs) {
    players.push_back(&pilot);
  }
  sort(players.begin(), players.end(), [](const std::string *a, const std::string *b) {
    return std::lexicographical_compare(a->begin(), a->end(), b->begin(), b->end(),
                                        [](char ac, char bc) { return std::tolower(ac) < std::tolower(bc); });
  });
  auto     it = std::max_element(players.begin(), players.end(),
                                 [](const std::string *a, const std::string *b) { return a->size() < b->size(); });
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

void TacFile::PrintAllPlayerRuns(std::ostream &strm) {
  for (auto it = pilot_runs.begin(); it != pilot_runs.end(); ++it) {
    for (const auto &plane : it->second) {
      plane->Print(strm, true, true, true, true, true);
      strm << "\n";
      for (const auto &mis : plane->getMissiles()) {
        strm << "\t";
        mis->Print(strm, true, true, false, false, false);
        strm << "\n";
      }
    }
  }
}

void TacFile::PrintPlayerRuns(std::ostream &strm, const std::string &username) {
  auto it = pilot_runs.find(username);
  if (it != pilot_runs.end()) {
    for (const auto &plane : it->second) {
      plane->Print(strm, true, true, true, true, true);
      strm << "\n";
      for (const auto &mis : plane->getMissiles()) {
        strm << "\t";
        mis->Print(strm, true, true, false, false, false);
        strm << "\n";
      }
    }
  } else {
    strm << "No data for " << username << "\n";
  }
}

bool TacFile::LoadFile(const std::filesystem::path &filepath) { return LoadFileWithChunks(filepath); }

#define AVG_CHARACTERS_PER_LINE 50

std::regex pattern = std::regex(
    R"((?:^|\n)(#?)(-?[0-9a-f.]+)(?:,T=([-\d.|]*)(?:$|,Type=((?:Weapon\+Missile|Air\+FixedWing)[^,]*),Name=([^,]*)(?:,Pilot=([^,]*))?(?:,Group=([^,]*))?,Color=(Blue|Red)[^\n]*)?)?(?=$|\n))",
    std::regex_constants::optimize);

static void progressThread(const std::vector<size_t> &prog,
                           const size_t               total_size,
                           size_t                    &lines_processed,
                           size_t                    &total_lines,
                           bool                      &done) {
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
      option::MaxProgress(total_size + total_lines),
  };

  bool file_parsed = false;

  while (true) {
    size_t current_prog = 0;
    size_t total_prog = total_size + total_lines;
    if (done) {
      current_prog = total_prog;
    } else {
      if (file_parsed) {
        current_prog += total_size;
      } else {
        for (const auto &p : prog) {
          current_prog += p;
        }
        if (current_prog >= total_size) {
          file_parsed = true;
        }
      }
      current_prog += lines_processed;
    }
    progress.set_option(option::MaxProgress(total_prog));
    progress.set_progress(static_cast<size_t>(current_prog));
    if (progress.is_completed() || done) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

bool TacFile::LoadFileWithChunks(const std::filesystem::path &filepath) {
  uint64_t                               extraction_time = 0;
  uint64_t                               file_read_time = 0;
  std::unique_ptr<std::filesystem::path> txt_file_path;
  std::string                            fileContents;
  MEASURE_TIME(extraction_time, txt_file_path = std::unique_ptr<std::filesystem::path>(PrepareACMIFile(filepath));)
  if (!txt_file_path.get()) {
    std::cerr << "Could not prepare file\n";
    return false;
  }
  std::cout << "Extraction time  : " << extraction_time / 1000000.0f << "s\n";
  std::string path = txt_file_path->string();

  MEASURE_TIME(
      file_read_time, std::ifstream file(path); if (!file.is_open()) {
        std::cerr << "Could not open extracted file\n";
        return false;
      }

      std::stringstream buffer;
      buffer.clear(); buffer << file.rdbuf(); fileContents = buffer.str();)

  std::cout << "File read time: " << file_read_time / 1000000.0f << "s\n";
  const auto               processor_count = std::thread::hardware_concurrency();
  const auto               chunk_size = fileContents.size() / processor_count;
  std::vector<std::thread> threads;
  threads.reserve(processor_count);
  std::vector<std::pair<size_t, size_t>>              chunks;
  std::vector<size_t>                                 progress(processor_count, 0);
  std::vector<std::vector<std::unique_ptr<FileLine>>> file_lines;
  chunks.reserve(processor_count);
  file_lines.reserve(processor_count);
  size_t prevEnd = 0;
  for (uint8_t i = 0; i < 2; i++) {
    prevEnd = fileContents.find('\n', prevEnd);
  }
  prevEnd++;
  for (size_t i = 0; i < processor_count; i++) {
    size_t nextEnd = (i + 1) * chunk_size;
    while (nextEnd < fileContents.size() && fileContents[nextEnd] != '\n') {
      nextEnd++;
    }
    nextEnd++;
    chunks.emplace_back(prevEnd, nextEnd);
    file_lines.emplace_back();
    file_lines.back().reserve(chunk_size / AVG_CHARACTERS_PER_LINE);
    prevEnd = nextEnd;
  }

  size_t total_size = fileContents.size() - chunks[0].first;
  size_t lines_processed = 0;
  size_t total_lines = total_size / AVG_CHARACTERS_PER_LINE;
  size_t real_lines = 0;
  bool   finished = false;

  std::thread progress_thread(progressThread, std::cref(progress), total_size, std::ref(lines_processed),
                              std::ref(total_lines), std::ref(finished));

  for (size_t i = 0; i < processor_count; i++) {
    threads.emplace_back(TacFile::AnalyseChunk, std::cref(fileContents), std::ref(file_lines[i]), chunks[i].first,
                         chunks[i].second, std::ref(progress[i]));
  }

  std::chrono::milliseconds cur_time;

  for (size_t i = 0; i < processor_count; i++) {
    threads[i].join();
    real_lines += file_lines[i].size();
    total_lines = real_lines + (processor_count - 1 - i) * (total_size / (processor_count * AVG_CHARACTERS_PER_LINE));
    for (const auto &line : file_lines[i]) {
      line->ProcessLine(active_objects, tracked_objects, pilot_runs, cur_time);
    }
    lines_processed += file_lines[i].size();
  }

  finished = true;
  progress_thread.join();
  return true;
}

void TacFile::AnalyseChunk(const std::string                      &fileContents,
                           std::vector<std::unique_ptr<FileLine>> &lines,
                           size_t                                  startIdx,
                           size_t                                  endIdx,
                           size_t                                 &progress) {
  std::sregex_iterator it(fileContents.begin() + startIdx, fileContents.begin() + endIdx, pattern);
  std::sregex_iterator end;
  while (it != end) {
    progress = it->position() + it->length();
    // Group 1: # Symbol (timestamp)
    // Group 2: ID / Time(seconds)
    // Group 3: Location String
    // Group 4: Type
    // Group 5: Name
    // Group 6: Pilot
    // Group 7: Group
    // Group 8: Color
    std::smatch match = *it;
    if (match[1].length() > 0) {
      // Its a timestamp
      lines.emplace_back(new TimestampLine(match[2].str()));
    } else {
      int32_t id = std::stoi(match[2].str(), nullptr, 16);
      if (id < 0) {
        // Its a despawn
        lines.emplace_back(new DespawnLine(-id));
      } else if (match[4].length() > 0) {
        // Its a spawn
        lines.emplace_back(new SpawnLine(id, match[3].str(), match[5].str(), match[4].str(), match[6].str(),
                                         match[7].str(), match[8].str()));
      } else {
        // Its an update
        lines.emplace_back(new UpdateLine(id, match[3].str()));
      }
    }
    ++it;
  }
}
