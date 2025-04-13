#include "tac_file.hpp"

#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <thread>

#include "logging.hpp"
#include "utils.hpp"

static std::shared_ptr<spdlog::logger> logger = initialize_logger("TacFile", spdlog::level::trace);
static std::regex                      double_extension(R"(\.[\w]+\.[\w]+$)");

std::filesystem::path *TacFile::PrepareACMIFile(const std::filesystem::path &filepath) {
  if (!std::filesystem::exists(filepath)) {
    logger->error("File does not exist: {}", filepath.generic_string());
    return nullptr;
  }
  std::smatch match;
  std::string filepath_str = filepath.generic_string();
  if (std::regex_search(filepath_str, match, double_extension)) {
    if (match[0].str() == ".zip.acmi") {
      // Need to unzip it
      std::filesystem::path base_path = std::filesystem::temp_directory_path() / "TacView_Analyser" / getRandomGUID();
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
      logger->info("File extension was not recognised: {}", match[0].str());
      return nullptr;
    }
  } else {
    logger->info("File extension was not recognised: {}", filepath_str);
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

bool TacFile::LoadFile(const std::filesystem::path &filepath, size_t &cur_prog, size_t &max_prog) {
  return LoadFileWithChunks(filepath, cur_prog, max_prog);
}

#define AVG_CHARACTERS_PER_LINE 50

static void progressThread(const std::vector<Thread_Payload> &thread_payloads,
                           const size_t                      &lines_processed,
                           size_t                            &current_progress,
                           size_t                            &total_progress) {
  while (true) {
    size_t total_prog = 0;
    size_t cur_prog = lines_processed;
    for (const auto &payload : thread_payloads) {
      size_t est_remaining = (payload.endIdx - payload.startIdx - payload.progress) / AVG_CHARACTERS_PER_LINE;
      cur_prog += payload.lines.size();
      total_prog += 2 * (payload.lines.size() + est_remaining);  // 2 for each line (one parsing and one processing)
    }
    current_progress = cur_prog;
    total_progress = total_prog;
    if (current_progress >= total_progress) break;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

bool TacFile::LoadFileWithChunks(const std::filesystem::path &filepath, size_t &cur_prog, size_t &max_prog) {
  uint64_t                               extraction_time = 0;
  uint64_t                               file_read_time = 0;
  std::unique_ptr<std::filesystem::path> txt_file_path;
  std::string                            fileContents;
  MEASURE_TIME(extraction_time, txt_file_path = std::unique_ptr<std::filesystem::path>(PrepareACMIFile(filepath));)
  if (!txt_file_path.get()) {
    logger->critical("Failed to prepare file: {}", filepath.generic_string());
    return false;
  }
  logger->debug("Extraction time: {}ms", extraction_time / 1000.0f);
  std::string path = txt_file_path->string();

  MEASURE_TIME(
      file_read_time, std::ifstream file(path); if (!file.is_open()) {
        logger->error("Could not open extracted file: {}", path);
        return false;
      }

      std::stringstream buffer;
      buffer.clear(); buffer << file.rdbuf(); fileContents = buffer.str();)

  logger->debug("File read time: {}ms", file_read_time / 1000.0f);
  const auto                  processor_count = std::thread::hardware_concurrency();
  const auto                  chunk_size = fileContents.size() / processor_count;
  std::vector<std::thread>    threads;
  std::vector<Thread_Payload> payloads;
  size_t                      prevEnd = 0;
  threads.reserve(processor_count);
  payloads.reserve(processor_count);

  logger->debug("Spawning {} threads to parse file", processor_count);
  // Skip the first two lines
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

    Thread_Payload payload = {
        .lines = std::vector<std::unique_ptr<FileLine>>(),
        .startIdx = prevEnd,
        .endIdx = nextEnd,
        .progress = 0,
        .threadNo = i
    };
    payload.lines.reserve(chunk_size / AVG_CHARACTERS_PER_LINE);
    payloads.emplace_back(std::move(payload));
    prevEnd = nextEnd;
  }

  size_t lines_processed = 0;

  std::thread progress_thread(progressThread, std::cref(payloads), std::cref(lines_processed), std::ref(cur_prog),
                              std::ref(max_prog));

  for (size_t i = 0; i < processor_count; i++) {
    threads.emplace_back(TacFile::AnalyseChunk, std::cref(fileContents), std::ref(payloads[i]));
  }

  std::chrono::milliseconds cur_time;

  for (size_t i = 0; i < processor_count; ++i) {
    threads[i].join();
    for (const auto &line : payloads[i].lines) {
      line->ProcessLine(active_objects, tracked_objects, pilot_runs, cur_time);
    }
    lines_processed += payloads[i].lines.size();
  }

  progress_thread.join();
  return true;
}

std::regex pattern = std::regex(
    R"((?:^|\n)(#?)(-?[0-9a-f.]+)(?:,T=([-\d.|]*)(?:$|,Type=((?:Weapon\+Missile|Air\+FixedWing)[^,]*),Name=([^,]*)(?:,Pilot=([^,]*))?(?:,Group=([^,]*))?,Color=(Blue|Red)[^\n]*)?)?(?=$|\n))",
    std::regex_constants::optimize);

void TacFile::AnalyseChunk(const std::string &fileContents, Thread_Payload &payload) {
  logger->trace("Starting thread {}: File chunk ({} -> {})", payload.threadNo, payload.startIdx, payload.endIdx);
  std::sregex_iterator it(fileContents.begin() + payload.startIdx, fileContents.begin() + payload.endIdx, pattern);
  std::sregex_iterator end;
  while (it != end) {
    payload.progress = it->position() + it->length() - payload.startIdx;
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
      payload.lines.emplace_back(new TimestampLine(match[2].str()));
    } else {
      int32_t id = std::stoi(match[2].str(), nullptr, 16);
      if (id < 0) {
        // Its a despawn
        payload.lines.emplace_back(new DespawnLine(-id));
      } else if (match[4].length() > 0) {
        // Its a spawn
        payload.lines.emplace_back(new SpawnLine(id, match[3].str(), match[5].str(), match[4].str(), match[6].str(),
                                                 match[7].str(), match[8].str()));
      } else {
        // Its an update
        payload.lines.emplace_back(new UpdateLine(id, match[3].str()));
      }
    }
    ++it;
  }
  payload.progress = payload.endIdx - payload.startIdx;
  logger->trace("Thread {}: Finished processing chunk", payload.threadNo);
}
