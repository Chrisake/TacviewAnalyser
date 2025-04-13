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

// Define a macro for each log level.  These make logging *much* easier.
#define LOG_TRACE(logger, message, ...) \
  if (logger) logger->trace(message, ##__VA_ARGS__)
#define LOG_DEBUG(logger, message, ...) \
  if (logger) logger->debug(message, ##__VA_ARGS__)
#define LOG_INFO(logger, message, ...) \
  if (logger) logger->info(message, ##__VA_ARGS__)
#define LOG_WARN(logger, message, ...) \
  if (logger) logger->warn(message, ##__VA_ARGS__)
#define LOG_ERROR(logger, message, ...) \
  if (logger) logger->error(message, ##__VA_ARGS__)
#define LOG_CRITICAL(logger, message, ...) \
  if (logger) logger->critical(message, ##__VA_ARGS__)
