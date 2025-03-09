#include "cache.hpp"
#include "global.hpp"

#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

CacheObject::CacheObject() : username("") {}
CacheObject::CacheObject(std::string username) : username(username) {}

std::filesystem::path CacheObject::getDefaultFilePath() {
	return (exe_dir / "cache.json").string();
}

void CacheObject::LoadCache(const std::filesystem::path& filepath) {
	std::ifstream file;
	file.open(filepath, std::ifstream::in);
	if (!file.is_open()) {
		std::cerr << "Failed to open cache file: " << filepath << "\n";
		return;
	}
	nlohmann::json j;
	file >> j;
	username = j["username"];
}

void CacheObject::LoadCache() {
	LoadCache(getDefaultFilePath());
}

void CacheObject::SaveCache(const std::filesystem::path& filepath) {
	std::ofstream file;
	file.open(filepath, std::ofstream::out);
	if (!file.is_open()) {
		std::cerr << "Failed to open cache file: " << filepath << "\n";
		return;
	}
	nlohmann::json j;
	j["username"] = username;
	file << j.dump(4);
}

void CacheObject::SaveCache() {
	SaveCache(getDefaultFilePath());
}