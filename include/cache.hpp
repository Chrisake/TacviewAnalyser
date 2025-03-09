#pragma once
#include <string>
#include <filesystem>

class CacheObject {
private:
	std::filesystem::path getDefaultFilePath();
public:
	std::string username;
	CacheObject();
	CacheObject(std::string username);
	void LoadCache(const std::filesystem::path &filepath);
	void LoadCache();
	void SaveCache(const std::filesystem::path &filepath);
	void SaveCache();
};