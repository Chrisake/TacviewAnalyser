#pragma once
#include <chrono>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Missile;

class Location {
 protected:
  float longtitude;
  float latitude;
  float altitude;
  float roll;
  float pitch;
  float yaw;
  float x;
  float y;
  float heading;

 public:
  Location();
  static Location parseFromString(const std::string& str);
  void operator+=(const Location& rhs);
  float distance(const Location& rhs);
  bool similar(const Location& loc2);
};

typedef enum { RED, BLUE, NEUTRAL } coalition_t;

class SimObject {
 protected:
  uint32_t id;
  std::string name;
  std::chrono::milliseconds spawn_time;
  Location spawn_location;
  std::chrono::milliseconds despawn_time;
  Location despawn_location;
  Location last_location;
  std::chrono::milliseconds last_update;
  coalition_t coalition;
  const SimObject* collided_with;
 public:
  SimObject(const uint32_t id, const std::string& name,
            const std::chrono::milliseconds& time, const Location& location,
            const std::string& coalition);
  void update(const Location& new_location,
              const std::chrono::milliseconds& time);
  void despawn(const std::chrono::milliseconds& time);
  void collideWith(const SimObject* obj);
  const SimObject* getCollidedWith() const;
  bool check_collision(const SimObject* rhs);
  const Location& getLastLocation() const;
  const coalition_t& getCoalition() const;
  const std::string& getName() const;
  virtual const std::string& getPilotName() const;
  virtual bool launched_missile(Missile* rhs);
  void Print(std::ostream& strm, bool showTime = true,
             bool showCollision = false, bool showColor=true, bool showID=true, bool showPilot=true) const;
  friend std::ostream& operator<<(std::ostream& os, const SimObject& pl);
};

class Plane : public SimObject {
 protected:
  std::string pilot_name;
  std::string group;
  std::vector<Missile*> missiles;

 public:
  bool launched_missile(Missile* rhs) override;
  const std::string& getPilotName() const override;
  Plane(const uint32_t id, const std::string& name,
        const std::chrono::milliseconds& time, const Location& location,
        const std::string& coalition, const std::string& pilot_name,
        const std::string& grp);
  const std::vector<Missile*>& getMissiles();
};

class Missile : public SimObject {
 protected:
  const SimObject* parent;
 public:
  void launch(const SimObject* parent);
  const std::string& getPilotName() const override;
  Missile(const uint32_t id, const std::string& name, const std::chrono::milliseconds& time,
          const Location& location, const std::string& coalition);
};

class SamSite : public SimObject {
 protected:
  std::vector<Missile*> missiles;
 public:
  bool launched_missile(Missile* rhs) override;
  const std::vector<Missile*>& getMissiles();
};
