#include <string>

#include "sim_object.hpp"

class FileLine {
 protected:
  uint32_t id;

 public:
  FileLine(uint32_t id) : id(id) {}
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

  class UpdateLine : public FileLine {
    Location location;

   public:
    UpdateLine(uint32_t id, const std::string& loc)
        : FileLine(id), location(Location::parseFromString(loc)) {};
  };

  class DespawnLine : public FileLine {
   public:
    DespawnLine(uint32_t id) : FileLine(id) {};
  };

  class TimestampLine : public FileLine {
    std::chrono::milliseconds time;

   public:
    TimestampLine(const std::string& time)
        : FileLine(id), time(static_cast<uint64_t>(std::stof(time) * 1000)) {};
  };