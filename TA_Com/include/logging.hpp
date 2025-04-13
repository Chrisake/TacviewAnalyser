#pragma once

#include <fmt/chrono.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>
#include <memory>

void initialize_log_sinks();

[[nodiscard]] std::shared_ptr<spdlog::logger> initialize_logger(const std::string        &logger_name,
                                                                spdlog::level::level_enum level);

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