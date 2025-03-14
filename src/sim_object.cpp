#include "sim_object.hpp"

#include <limits>
#include <regex>

#include <string_view>
#include "utils.hpp"

#define PROXIMITY_LONG_THRESHOLD 0.003
#define PROXIMITY_LATI_THRESHOLD 0.003
#define PROXIMITY_ALTI_THRESHOLD 30

Location::Location()
    : longtitude(std::numeric_limits<float>::max()),
      latitude(std::numeric_limits<float>::max()),
      altitude(std::numeric_limits<float>::max()),
      roll(std::numeric_limits<float>::max()),
      pitch(std::numeric_limits<float>::max()),
      yaw(std::numeric_limits<float>::max()),
      x(std::numeric_limits<float>::max()),
      y(std::numeric_limits<float>::max()),
      heading(std::numeric_limits<float>::max()) {}

static inline float getParam(const char* p) {
  if (p == nullptr || *p == 0 || *p == '|')
    return std::numeric_limits<float>::max();
  return std::stof(p);
}

Location Location::parseFromString(const std::string& str) {
  Location result;
  const char* s = str.c_str();
  std::vector<const char*> tokens;
  tokens.reserve(9);
  tokens.push_back(s);
  while (*s) {
    if (*s == '|') {
      tokens.push_back(++s);
    } else {
      s++;
    }
  }

  if (tokens.size() == 3) {
    result.longtitude = getParam(tokens[0]);
    result.latitude = getParam(tokens[1]);
    result.altitude = getParam(tokens[2]);
  } else if (tokens.size() == 5) {
    result.longtitude = getParam(tokens[0]);
    result.latitude = getParam(tokens[1]);
    result.altitude = getParam(tokens[2]);
    result.x = getParam(tokens[3]);
    result.y = getParam(tokens[4]);
  } else if (tokens.size() == 6) {
    result.longtitude = getParam(tokens[0]);
    result.latitude = getParam(tokens[1]);
    result.altitude = getParam(tokens[2]);
    result.roll = getParam(tokens[3]);
    result.pitch = getParam(tokens[4]);
    result.yaw = getParam(tokens[5]);
  } else if (tokens.size() == 9) {
    result.longtitude = getParam(tokens[0]);
    result.latitude = getParam(tokens[1]);
    result.altitude = getParam(tokens[2]);
    result.roll = getParam(tokens[3]);
    result.pitch = getParam(tokens[4]);
    result.yaw = getParam(tokens[5]);
    result.x = getParam(tokens[6]);
    result.y = getParam(tokens[7]);
    result.heading = getParam(tokens[8]);
  }
  return result;
}

void Location::operator+=(const Location& rhs) {
  if (rhs.longtitude != std::numeric_limits<float>::max())
    longtitude = rhs.longtitude;
  if (rhs.latitude != std::numeric_limits<float>::max())
    latitude = rhs.latitude;
  if (rhs.altitude != std::numeric_limits<float>::max())
    altitude = rhs.altitude;
  if (rhs.roll != std::numeric_limits<float>::max()) roll = rhs.roll;
  if (rhs.pitch != std::numeric_limits<float>::max()) pitch = rhs.pitch;
  if (rhs.yaw != std::numeric_limits<float>::max()) yaw = rhs.yaw;
  if (rhs.x != std::numeric_limits<float>::max()) x = rhs.x;
  if (rhs.y != std::numeric_limits<float>::max()) y = rhs.y;
  if (rhs.heading != std::numeric_limits<float>::max()) heading = rhs.heading;
}

float Location::distance(const Location& rhs) {
  return sqrtf((x - rhs.x) * (x - rhs.x) + (y - rhs.y) * (y - rhs.y) +
               (altitude - rhs.altitude) * (altitude - rhs.altitude));
}

bool Location::similar(const Location& loc) {
  return (abs(longtitude - loc.longtitude) <= PROXIMITY_LONG_THRESHOLD &&
          abs(latitude - loc.latitude) <= PROXIMITY_LATI_THRESHOLD &&
          abs(altitude - loc.altitude) <= PROXIMITY_ALTI_THRESHOLD);
}

void SimObject::update(const Location& new_location,
                       const std::chrono::milliseconds& time) {
  last_location += new_location;
  last_update = time;
}

bool SimObject::check_collision(const SimObject* rhs) {
  return this != rhs && last_location.similar(rhs->last_location);
}

Plane::Plane(const uint32_t id, const std::string& name,
             const std::chrono::milliseconds& time, const Location& location,
             const std::string& coalition, const std::string& pilot_name,
             const std::string& grp)
    : SimObject(id, name, time, location, coalition),
      pilot_name(pilot_name),
      group(grp) {}

SimObject::SimObject(const uint32_t id, const std::string& name,
                     const std::chrono::milliseconds& time,
                     const Location& location, const std::string& coal)
    : id(id),
      name(name),
      spawn_time(time),
      spawn_location(location),
      despawn_time(-1),
      despawn_location(),
      last_update(time),
      last_location(location),
      collided_with(nullptr) {
  if (coal == "Red") {
    this->coalition = RED;
  } else if (coal == "Blue") {
    this->coalition = BLUE;
  } else {
    this->coalition = NEUTRAL;
  }
}

std::ostream& operator<<(std::ostream& os, const SimObject& obj) {
  printDuration(obj.spawn_time, os);
  os << " -> ";
  printDuration(obj.despawn_time, os);
  os << " [" << obj.id << "] " << obj.name;
  return os;
}

void SimObject::Print(std::ostream& strm, bool showTime, bool showCollision,
                      bool showColor, bool showID, bool showPilot) const {
  if (showTime) {
    printDuration(spawn_time, strm);
    strm << " -> ";
    printDuration(despawn_time, strm);
    strm << " ";
  }
  if (showID) {
    strm << "[" << id << "] ";
  }
  strm << name << " ";
  if (showPilot) {
    strm << "(" << getPilotName() << ") ";
  }
  if (showColor) {
    strm << (this->coalition == RED
                 ? "Red"
                 : (this->coalition == BLUE ? "Blue" : "Neutral"))
         << " ";
  }
  if (showCollision && collided_with) {
    strm << "-- [ ";
    collided_with->Print(strm, false, false, true, false, true);
    strm << "] ";
  }
}

const Location& SimObject::getLastLocation() const { return last_location; }

bool SimObject::launched_missile([[maybe_unused]] Missile* rhs) {
  return false;
}

const std::string& SimObject::getName() const { return name; }

const std::string& SimObject::getPilotName() const {
  static const std::string default_pilot = " - ";
  return default_pilot;
}

const std::string& Plane::getPilotName() const { return pilot_name; }

const std::string& Missile::getPilotName() const {
  static const std::string default_pilot = "unknown";
  if (parent)
    return parent->getPilotName();
  else
    return default_pilot;
}

bool Plane::launched_missile(Missile* rhs) {
  bool result = this->coalition == rhs->getCoalition() &&
                last_location.similar(rhs->getLastLocation());
  if (result) {
    missiles.push_back(rhs);
    rhs->launch(this);
  }
  return result;
}

bool SamSite::launched_missile(Missile* rhs) {
  bool result = this->coalition == rhs->getCoalition() &&
                last_location.similar(rhs->getLastLocation());
  if (result) {
    missiles.push_back(rhs);
    rhs->launch(this);
  }
  return result;
}

void SimObject::despawn(const std::chrono::milliseconds& time) {
  despawn_location = last_location;
  despawn_time = time;
}

const coalition_t& SimObject::getCoalition() const { return coalition; }

Missile::Missile(const uint32_t id, const std::string& name,
                 const std::chrono::milliseconds& time,
                 const Location& location, const std::string& coalition)
    : SimObject(id, name, time, location, coalition), parent(nullptr) {}

void Missile::launch(const SimObject* prnt) { parent = prnt; }

const std::vector<Missile*>& Plane::getMissiles() { return missiles; }

const std::vector<Missile*>& SamSite::getMissiles() { return missiles; }

void SimObject::collideWith(const SimObject* obj) { collided_with = obj; }

const SimObject* SimObject::getCollidedWith() const { return collided_with; }