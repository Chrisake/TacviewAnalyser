#pragma once

#include <minizip/unzip.h>

#include <filesystem>
#include <string>
#include <string_view>


/*
 * @brief Macro to measure the execution time of a code block.
 *
 * @param variable Name of the variable to store the duration
 * @param code_block Code block to measure
 */
#define MEASURE_TIME(variable, code_block)                                                      \
  {                                                                                             \
    auto start = std::chrono::high_resolution_clock::now();                                     \
    code_block;                                                                                 \
    auto end = std::chrono::high_resolution_clock::now();                                       \
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(); \
    variable += duration;                                                                       \
  }

/*
 * @brief Unzips a file from the origin path to the destination path.
 *
 * @param origin Path to the zip file to unzip
 * @param destination Path to the destination directory
 *
 * @return 0 on success, non-zero on failure
 */
int unzip_file(const std::filesystem::path &origin, const std::filesystem::path &destination);

/*
 * @brief Outputs the duration in a human-readable format on the given stream.
 *
 * @param duration Duration to output
 * @param strm Output stream
 */
void printDuration(std::chrono::milliseconds duration, std::ostream &strm);

/*
 * @brief Generates a random string of the given length using the specified characters.
 *
 * @param length Length of the string to generate
 * @param characters Characters to include in the string
 *
 * @return Random string of the given length
 */
std::string getRandomString(size_t length, std::string_view characters);

/*
 * @brief Generates a random string of the given length using alphanumeric characters. (abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-_=+'\";:,.<>/?)
 *
 * @param length Length of the string to generate
 *
 * @return Random string of the given length
 */ 
std::string getRandomSring(size_t length);

/*
 * @brief Generates a random GUID (Globally Unique Identifier).
 *
 * @return Random GUID as a string
 */
std::string getRandomGUID();
