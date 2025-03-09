#pragma once

#include <minizip/unzip.h>
#include <filesystem>

#define MEASURE_TIME(variable, code_block)                                 \
  {                                                                        \
    auto start = std::chrono::high_resolution_clock::now();                \
    code_block;                                                            \
    auto end = std::chrono::high_resolution_clock::now();                  \
    auto duration =                                                        \
        std::chrono::duration_cast<std::chrono::microseconds>(end - start) \
            .count();                                                      \
    variable += duration;                                                  \
  }

int unzip_file(const std::filesystem::path &origin,
               const std::filesystem::path &destination);

void printDuration(std::chrono::milliseconds duration, std::ostream &strm);
