#pragma once
#include <filesystem>
#include <memory>
#include <unordered_map>

#include "file_line.hpp"
#include "sim_object.hpp"

struct Thread_Payload {
  std::vector<std::unique_ptr<FileLine>> lines;
  size_t                                 startIdx;
  size_t                                 endIdx;
  size_t                                 progress;
  size_t                                 threadNo;
};

class TacFile {
 private:
  static std::filesystem::path *PrepareACMIFile(const std::filesystem::path &filepath);
  static void                   AnalyseChunk(const std::string &fileContents, Thread_Payload &payload);
  bool LoadFileWithChunks(const std::filesystem::path &filepath, size_t &cur_prog, size_t &max_prog);

 protected:
  std::unordered_map<std::string, std::string>          server_info;
  std::vector<std::unique_ptr<SimObject>>               tracked_objects;
  std::unordered_map<uint32_t, SimObject *>             active_objects;
  std::unordered_map<std::string, std::vector<Plane *>> pilot_runs;

 public:
  TacFile() = default;

  bool LoadFile(const std::filesystem::path &filepath, size_t &cur_prog, size_t &max_prog);

  void PrintPlayers(std::ostream &strm);
  void PrintAllPlayerRuns(std::ostream &strm);
  void PrintPlayerRuns(std::ostream &strm, const std::string &username);
};
