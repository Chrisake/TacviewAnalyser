#include "logging.hpp"

bool                          log_sinks_initialized = false;
std::vector<spdlog::sink_ptr> log_sinks;

void initialize_log_sinks() {
  if (log_sinks_initialized) return;
  std::time_t t = std::time(nullptr);
  std::string formatted_time = fmt::format("{:%d-%m-%Y_%H-%M-%S}", fmt::localtime(t));
  std::string filename = fmt::format("log/tacview_analyser_{}.log", formatted_time);
  size_t      max_size = 10 * 1024 * 1024;  // 10MB
  int         max_files = 100;
  log_sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());  // Colored console
  log_sinks.push_back(
      std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filename, max_size, max_files));  // Rotating file

  log_sinks_initialized = true;
}

[[nodiscard]] std::shared_ptr<spdlog::logger> initialize_logger(const std::string        &logger_name,
                                                                spdlog::level::level_enum level) {
  if (!log_sinks_initialized) initialize_log_sinks();
  // Create the logger
  auto res = std::make_shared<spdlog::logger>(logger_name, log_sinks.begin(), log_sinks.end());
  // Set the log level (default to debug, can be changed)
  res->set_level(level);
  // Optionally set a default format
  res->set_pattern("%H:%M:%S.%e [%^%l%$] (TID:%t) | %n: %v");
  spdlog::register_logger(res);
  return res;
}
