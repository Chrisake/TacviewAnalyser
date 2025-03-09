#include <fstream>
#include <indicators/progress_bar.hpp>
#include <iostream>
#include <memory>
#include <regex>
#include <string>

#include "tac_file.hpp"
#include "utils.hpp"

// static std::regex line(
//    R"(T=([-\d.|]*)(?:,Type=([^,]*))?(?:,Name=([^,]*))?(?:,Pilot=([^,]*))?(?:,Group=([^,]*))?(?:,Color=([^,]*))?(?:,Coalition=([^,]*))?(?:,Country=([^,]*))?$)");
static std::regex line(R"(T=([-\d.|]*)(?:$|,Type=((?:Weapon\+Missile|Air\+FixedWing)[^,]*),Name=([^,]*)(?:,Pilot=([^,]*))?(?:,Group=([^,]*))?,Color=(Blue|Red)))");
static std::regex field(R"(([\w]*)=((?:[^,]|\\,)*?[^\\])(?:\n|$|,))");


static void progressThread(std::ifstream& file, const uint64_t fileSize) {
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
      option::MaxProgress(fileSize),
  };

  while (true) {
    progress.set_progress(static_cast<size_t>(file.tellg()));
    if (progress.is_completed()) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

void TacFile::LoadFileOptimized(const std::filesystem::path& filepath) {
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

  uint64_t cnt = 0;
  std::chrono::milliseconds cur_time;
  std::string buffer;
  std::string line_buffer;
  std::vector<uint32_t> ids_to_erase;
  std::thread t(progressThread, std::ref(file), fileSize);
  uint64_t load_time = 0;
  uint64_t regex_time = 0;
  std::chrono::steady_clock::time_point begin_tot =
      std::chrono::steady_clock::now();
  while (true) {
    MEASURE_TIME(
        load_time, std::getline(file, line_buffer);
        if (line_buffer.ends_with('\\')) {
          buffer += line_buffer.substr(0, line_buffer.size() - 1);
          buffer += '\n';
          continue;
        } else { buffer += line_buffer; } if (!file) break;);
    cnt++;
    if (buffer[0] == '#') {
      for (const auto& id : ids_to_erase) {
        active_objects.erase(id);
      }
      ids_to_erase.clear();
      cur_time = std::chrono::milliseconds(
          static_cast<uint64_t>(std::stof(&buffer[1])) * 1000);
    } else {
      int32_t id = cnt <= 2 ? 0 : std::stoi(buffer, nullptr, 16);

      if (id == 0) {

        // It is a server object
        /* Ignore them for efficiency
        std::smatch match;
        auto s = buffer.cbegin();
        while (std::regex_search(s, buffer.cend(), match, field)) {
          server_info[match[1].str()] = match[2].str();
          s = match.suffix().first;
        }
        */
      } else if (id < 0) {
        // Despawning Object
        id = -id;
        auto it = active_objects.find(id);
        if (it != active_objects.end()) {
          it->second->despawn(cur_time);
          SimObject* obj = it->second;
          ids_to_erase.push_back(id);

          if (!obj->getCollidedWith()) {
            for (auto& a_obj : active_objects) {
              if (a_obj.second->check_collision(obj)) {
                obj->collideWith(a_obj.second);
                a_obj.second->collideWith(obj);
                break;
              }
            }
          }
        }
      } else {
        std::smatch match;
        bool res; 
        MEASURE_TIME(regex_time, res = std::regex_search(buffer, match, line););
        if (res) {
          const std::string& location = match[1].str();
          const std::string& type = match[2].str();
          const std::string& name = match[3].str();
          const std::string& pilot = match[4].str();
          const std::string& group = match[5].str();
          const std::string& color = match[6].str();
          const std::string& coalition = match[7].str();
          const std::string& country = match[8].str();

          if (type.empty()) {
            auto it = active_objects.find(id);
            if (it != active_objects.end()) {
              it->second->update(Location::parseFromString(location), cur_time);
            }
          } else {
            SimObject* obj = nullptr;
            if (type == "Air+FixedWing") {
              auto new_plane = new Plane(id, name, cur_time,
                                         Location::parseFromString(location),
                                         color, pilot, group);
              auto it = pilot_runs.find(pilot);
              if (it == pilot_runs.end()) {
                pilot_runs[pilot] = std::vector<Plane*>(1, new_plane);
              } else {
                it->second.emplace_back(new_plane);
              }
              obj = new_plane;
            } else if (type == "Weapon+Missile") {
              auto new_missile =
                  new Missile(id, name, cur_time,
                              Location::parseFromString(location), color);
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
          }
        }
      }
    }
    buffer.resize(0);
  }
  std::chrono::steady_clock::time_point end_tot =
      std::chrono::steady_clock::now();
  uint64_t total_time =
      std::chrono::duration_cast<std::chrono::microseconds>(end_tot - begin_tot)
          .count();
  t.join();
  std::cout << "Load Time: " << (100.0f * load_time) / total_time << "\n";
  std::cout << "Regex Time: " << (100.0f * regex_time) / total_time << "\n";
}