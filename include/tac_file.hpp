#pragma once
#include <filesystem>
#include <memory>
#include <unordered_map>

#include "file_line.hpp"
#include "sim_object.hpp"

class TacFile {
 private:
  static std::filesystem::path* PrepareACMIFile(
      const std::filesystem::path& filepath);
  static void AnalyseChunk(const std::string& fileContents,
                           std::vector<std::unique_ptr<FileLine>>& lines,
                           size_t startIdx, size_t endIdx, size_t &progress);
  void LoadFileWithChunks(const std::filesystem::path& filepath);

 protected:
  std::unordered_map<std::string, std::string> server_info;
  std::vector<std::unique_ptr<SimObject>> tracked_objects;
  std::unordered_map<uint32_t, SimObject*> active_objects;
  std::unordered_map<std::string, std::vector<Plane*>> pilot_runs;

 public:
  TacFile() = default;

  void LoadFile(const std::filesystem::path& filepath);

  void PrintPlayers(std::ostream& strm);
  void PrintAllPlayerRuns(std::ostream& strm);
  void PrintPlayerRuns(std::ostream& strm, const std::string& username);
};
