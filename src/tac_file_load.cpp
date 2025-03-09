#include <fstream>
#include <indicators/progress_bar.hpp>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <thread>

#include "tac_file.hpp"
#include "utils.hpp"

static std::regex line(R"(^(?:([\da-f]*),([\w\W]*)|#([\d.]*)|-([\da-f]*))$)");
static std::regex field(R"(([\w]*)=((?:[^,]|\\,)*?[^\\])(?:\n|$|,))");

void TacFile::LoadFileWithChunks(const std::filesystem::path& filepath) {
  std::unique_ptr<std::filesystem::path> txt_file_path(
      PrepareACMIFile(filepath));

  if (!txt_file_path.get()) {
    std::cerr << "Could not prepare file\n";
    return;
  }

  std::string path = txt_file_path->string();
  uint64_t fileSize = std::filesystem::file_size(*txt_file_path);

  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Could not open extracted file\n";
    return;
  }

  std::stringstream buffer;
  buffer.clear();
  buffer << file.rdbuf();
  std::string fileContents = buffer.str();

  const auto processor_count = std::thread::hardware_concurrency();
  const auto chunk_size = fileContents.size() / processor_count;
  std::vector<std::thread> threads;
  threads.reserve(processor_count);
  std::vector<std::pair<size_t, size_t>> chunks;
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
    file_lines.back().reserve(chunk_size / 50);
    prevEnd = nextEnd;
  }

  for (size_t i = 4; i < 5; i++) {
    threads.emplace_back(TacFile::AnalyseChunk, std::cref(fileContents),
                         std::ref(file_lines[i]), chunks[i].first,
                         chunks[i].second);
  }

  for (auto& t : threads) {
    t.join();
  }
}

std::regex pattern = std::regex(
    R"((?:^|\n)(#?)(-?[0-9a-f.]+)(?:,T=([-\d.|]*)(?:$|,Type=((?:Weapon\+Missile|Air\+FixedWing)[^,]*),Name=([^,]*)(?:,Pilot=([^,]*))?(?:,Group=([^,]*))?,Color=(Blue|Red)[^\n]*)?)?(?=$|\n))",
    std::regex_constants::ECMAScript);

void TacFile::AnalyseChunk(const std::string& fileContents,
                           std::vector<std::unique_ptr<FileLine>>& lines,
                           size_t startIdx, size_t endIdx) {
  std::sregex_iterator it(fileContents.begin() + startIdx,
                          fileContents.begin() + endIdx, pattern);
  std::sregex_iterator end;
  while (it != end) {
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
      lines.emplace_back(match[2].str());
    } else {
      int32_t id = std::stoi(match[2].str(), nullptr, 16);
      if (id < 0) {
        // Its a despawn
        lines.emplace_back(new DespawnLine(-id));
      } else if (match[4].length() > 0) {
        // Its a spawn
        lines.emplace_back(new SpawnLine(id, match[3].str(), match[5].str(),
                                           match[4].str(), match[6].str(),
                                           match[7].str(), match[8].str()));
      } else {
        // Its an update
        lines.emplace_back(new UpdateLine(id, match[3].str()));
      }
    }
    ++it;
  }
}

void TacFile::LoadFileSimple(const std::filesystem::path& filepath) {
  using namespace indicators;
  std::unique_ptr<std::filesystem::path> txt_file_path(
      PrepareACMIFile(filepath));

  if (txt_file_path == nullptr) {
    std::cerr << "Could not prepare file\n";
    return;
  }

  std::string path = txt_file_path->string();
  uint64_t fileSize = std::filesystem::file_size(*txt_file_path);
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Could not open extracted file\n";
    return;
  }

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
      option::MaxProgress(fileSize),
  };

  std::chrono::milliseconds cur_time;
  std::string buffer;
  std::string line_buffer;
  std::vector<uint32_t> ids_to_erase;
  int cnt = 0;
  while (std::getline(file, line_buffer)) {
    if ((cnt & 0xFFFF) == 0) {
      progress.set_progress(static_cast<size_t>(file.tellg()));
    }
    cnt++;
    if (line_buffer.ends_with('\\')) {
      buffer += line_buffer.substr(0, line_buffer.size() - 1);
      buffer += '\n';
      continue;
    } else {
      buffer += line_buffer;
    }
    if (cnt <= 2) {
      std::smatch match;
      if (std::regex_search(buffer, match, field)) {
        server_info[match[1].str()] = match[2].str();
      }
    } else {
      std::smatch match;
      if (std::regex_search(buffer, match, line)) {
        /* Data Groups :
         *	1: ID
         *	2: Data
         *	3: Time
         *	4: DID
         */
        if (match[3].length() > 0) {
          for (const auto& id : ids_to_erase) {
            active_objects.erase(id);
          }
          ids_to_erase.clear();
          cur_time = std::chrono::milliseconds(
              static_cast<uint64_t>(std::stof(match[3].str()) * 1000));
        }
        if (match[4].length() > 0) {
          uint32_t id = std::stoi(match[4].str(), nullptr, 16);
          auto it = active_objects.find(id);
          if (it != active_objects.end()) {
            it->second->despawn(cur_time);
            SimObject* obj = it->second;
            ids_to_erase.push_back(id);
            // active_objects.erase(it);
            Missile* mis = dynamic_cast<Missile*>(obj);
            if (mis != nullptr) {
              for (auto& a_obj : active_objects) {
                if (a_obj.second->check_collision(mis)) {
                  mis->collideWith(a_obj.second);
                  a_obj.second->collideWith(mis);
                  break;
                }
              }
            }
          }
        }
        if (match[1].length() > 0) {
          uint32_t id = std::stoi(match[1].str(), nullptr, 16);
          std::smatch field_match;
          std::unordered_map<std::string, std::string> fields;
          std::string data = match[2].str();
          auto s = data.cbegin();
          while (std::regex_search(s, data.cend(), field_match, field)) {
            fields[field_match[1].str()] = field_match[2].str();
            s = field_match.suffix().first;
          }
          if (id == 0) {
            // It is a server object
            server_info.insert(fields.begin(), fields.end());
          } else if (fields.find("Type") != fields.end()) {
            // It is a spawn Object
            SimObject* obj = nullptr;
            if (fields.at("Type") == "Air+FixedWing") {
              auto new_plane = new Plane(
                  id, fields.at("Name"), cur_time,
                  Location::parseFromString(fields.at("T")), fields.at("Color"),
                  fields.at("Pilot"), fields.at("Group"));
              auto it = pilot_runs.find(fields.at("Pilot"));
              if (it == pilot_runs.end()) {
                pilot_runs[fields.at("Pilot")] =
                    std::vector<Plane*>(1, new_plane);
              } else {
                it->second.emplace_back(new_plane);
              }
              obj = new_plane;
            } else if (fields.at("Type") == "Weapon+Missile") {
              auto new_missile =
                  new Missile(id, fields.at("Name"), cur_time,
                              Location::parseFromString(fields.at("T")),
                              fields.at("Color"));
              for (auto& a_obj : active_objects) {
                if (a_obj.second->launched_missile(new_missile)) {
                  break;
                }
              }
              obj = new_missile;
            }
            if (obj != nullptr) {
              tracked_objects.emplace_back(obj);
              active_objects[id] = obj;
            }
          } else {
            // It is an update for an object
            auto it = active_objects.find(id);
            if (it != active_objects.end()) {
              it->second->update(Location::parseFromString(fields.at("T")),
                                 cur_time);
            }
          }
        }
      }
    }
    buffer.resize(0);
  }
}