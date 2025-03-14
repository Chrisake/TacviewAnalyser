#include <string>

#include "sim_object.hpp"

class FileLine {
 protected:
  uint32_t id;

 public:
  FileLine(uint32_t id) : id(id) {};
  virtual ~FileLine() = default;
  virtual void ProcessLine(
      std::unordered_map<uint32_t, SimObject*>& active_objects,
      std::vector<std::unique_ptr<SimObject>>& tracked_objects,
      std::unordered_map<std::string, std::vector<Plane*>>& pilot_runs,
      std::chrono::milliseconds& cur_time) const = 0;
};

class SpawnLine : public FileLine {
  Location location;
  std::string name;
  std::string type;
  std::string pilot;
  std::string group;
  std::string color;

 public:
  SpawnLine(uint32_t id, const std::string& loc, const std::string& name,
            const std::string& type, const std::string& pilot,
            const std::string& group, const std::string& color)
      : FileLine(id),
        location(Location::parseFromString(loc)),
        name(name),
        type(type),
        pilot(pilot),
        group(group),
        color(color) {};
  void ProcessLine(
      [[maybe_unused]] std::unordered_map<uint32_t, SimObject*>& active_objects,
      [[maybe_unused]] std::vector<std::unique_ptr<SimObject>>& tracked_objects,
      [[maybe_unused]] std::unordered_map<std::string, std::vector<Plane*>>& pilot_runs,
      [[maybe_unused]] std::chrono::milliseconds& cur_time) const {
    SimObject* obj = nullptr;
    if (type == "Air+FixedWing") {
      auto new_plane =
          new Plane(id, name, cur_time, location, color, pilot, group);
      auto it = pilot_runs.find(pilot);
      if (it == pilot_runs.end()) {
        pilot_runs[pilot] = std::vector<Plane*>(1, new_plane);
      } else {
        it->second.emplace_back(new_plane);
      }
      obj = new_plane;
    } else if (type == "Weapon+Missile") {
      auto new_missile = new Missile(id, name, cur_time, location, color);
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
};

class UpdateLine : public FileLine {
  Location location;

 public:
  UpdateLine(uint32_t id, const std::string& loc)
      : FileLine(id), location(Location::parseFromString(loc)) {};
  void ProcessLine(
      [[maybe_unused]] std::unordered_map<uint32_t, SimObject*>& active_objects,
      [[maybe_unused]] std::vector<std::unique_ptr<SimObject>>& tracked_objects,
      [[maybe_unused]] std::unordered_map<std::string, std::vector<Plane*>>&
          pilot_runs,
      [[maybe_unused]] std::chrono::milliseconds& cur_time) const {
    auto it = active_objects.find(id);
    if (it != active_objects.end()) {
      it->second->update(location, cur_time);
    }
  }
};

class DespawnLine : public FileLine {
 public:
  DespawnLine(uint32_t id) : FileLine(id) {};
  void ProcessLine(
      [[maybe_unused]] std::unordered_map<uint32_t, SimObject*>& active_objects,
      [[maybe_unused]] std::vector<std::unique_ptr<SimObject>>& tracked_objects,
      [[maybe_unused]] std::unordered_map<std::string, std::vector<Plane*>>&
          pilot_runs,
      [[maybe_unused]] std::chrono::milliseconds& cur_time) const {
    auto it = active_objects.find(id);
    if (it != active_objects.end()) {
      it->second->despawn(cur_time);
      SimObject* obj = it->second;
      active_objects.erase(it);
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
  }
};

class TimestampLine : public FileLine {
  std::chrono::milliseconds time;

 public:
  TimestampLine(const std::string& time)
      : FileLine(0), time(static_cast<uint64_t>(std::stof(time) * 1000)) {};
  void ProcessLine(
      [[maybe_unused]] std::unordered_map<uint32_t, SimObject*>& active_objects,
      [[maybe_unused]] std::vector<std::unique_ptr<SimObject>>& tracked_objects,
      [[maybe_unused]] std::unordered_map<std::string, std::vector<Plane*>>& pilot_runs,
      [[maybe_unused]] std::chrono::milliseconds& cur_time) const {
    cur_time = time;
  }
};