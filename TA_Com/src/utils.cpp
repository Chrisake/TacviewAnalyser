#include "utils.hpp"
#include <string>
#include <cstdlib>


int unzip_file(const std::filesystem::path &origin,
               const std::filesystem::path &destination) {
  std::string orig = origin.string();
  std::string dest = destination.string();
  unzFile uf = unzOpen(orig.c_str());
  if (uf == NULL) {
    printf("Cannot open %s\n", orig.c_str());
    return -1;
  }

  if (unzGoToFirstFile(uf) != UNZ_OK) {
    printf("Error going to first file\n");
    unzClose(uf);
    return -1;
  }

  do {
    char filename[256];
    if (unzGetCurrentFileInfo64(uf, NULL, filename, sizeof(filename), NULL, 0,
                                NULL, 0) != UNZ_OK) {
      printf("Error getting file info\n");
      unzClose(uf);
      return -1;
    }

    if (unzOpenCurrentFile(uf) != UNZ_OK) {
      printf("Error opening file %s\n", filename);
      unzClose(uf);
      return -1;
    }

    std::filesystem::create_directories(dest);
    std::string filepath = (dest / std::filesystem::path(filename)).string();

    FILE *f;
    errno_t err = fopen_s(&f, filepath.c_str(), "wb");
    if (err || f == nullptr) {
      printf("Cannot open destination file %s\n", filepath.c_str());
      unzCloseCurrentFile(uf);
      unzClose(uf);
      return -1;
    }

    int bytes_read;
    char buffer[8192];
    do {
      bytes_read = unzReadCurrentFile(uf, buffer, sizeof(buffer));
      if (bytes_read < 0) {
        printf("Error reading file %s\n", filename);
        fclose(f);
        unzCloseCurrentFile(uf);
        unzClose(uf);
        return -1;
      }

      if (bytes_read > 0) {
        fwrite(buffer, bytes_read, 1, f);
      }
    } while (bytes_read > 0);

    fclose(f);
    unzCloseCurrentFile(uf);
  } while (unzGoToNextFile(uf) == UNZ_OK);

  unzClose(uf);
  return 0;
}

void printDuration(std::chrono::milliseconds duration, std::ostream &strm) {
  // Calculate hours, minutes, seconds, and remaining milliseconds
  auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
  duration -= hours;
  auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
  duration -= minutes;
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
  duration -= seconds;

  // Print the formatted output
  strm << std::right;
  strm << std::setw(2) << std::setfill('0') << hours.count() << ":"
       << std::setw(2) << std::setfill('0') << minutes.count() << ":"
       << std::setw(2) << std::setfill('0') << seconds.count();
}

std::string getRandomString(size_t length, std::string_view characters) { std::string result;
  result.resize(length);
  for (size_t i = 0; i < length; ++i) {
    result[i] = characters[std::rand() % characters.size()];
  }
  return result;
}

std::string getRandomSring(size_t length) {
  return getRandomString(length, "abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-_=+'\\\";:,.<>/?");
}

std::string getRandomGUID() { return getRandomString(36, "0123456789abcdef"); }