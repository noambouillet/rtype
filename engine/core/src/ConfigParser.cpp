/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Generic key-value configuration file parser
*/

#include "engine/core/ConfigParser.hpp"

#include <filesystem>
#include <fstream>
#include <string_view>

#include "engine/core/Error.hpp"
#include "engine/core/Logger.hpp"

namespace engine::core {

std::string_view ConfigParser::trim(std::string_view value)
{
	const std::size_t first = value.find_first_not_of(" \t\r\n");
	if (first == std::string_view::npos) {
		return {};
	}
	const std::size_t last = value.find_last_not_of(" \t\r\n");
	return value.substr(first, last - first + 1);
}

std::map<std::string, std::string> ConfigParser::parseFile(const std::string& filepath)
{
	bool fileExists = false;
	try {
		fileExists = std::filesystem::exists(filepath);
	} catch (const std::filesystem::filesystem_error& error) {
		core::Logger::defaultLogger().error("ConfigParser: Filesystem error while checking " + filepath + ": " + error.what());
		throw ConfigError("Unable to check configuration file: " + std::string(error.what()));
	}
	if (!fileExists) {
		core::Logger::defaultLogger().debug("ConfigParser: Configuration file does not exist: " + filepath);
		return {};
	}
	std::ifstream file(filepath);
	if (!file.is_open()) {
		core::Logger::defaultLogger().error("ConfigParser: Unable to open configuration file: " + filepath);
		throw ConfigError("Unable to open configuration file: " + filepath);
	}
	std::map<std::string, std::string> data;
	std::string line;
	while (std::getline(file, line)) {
		const std::string_view content = trim(line);
		if (content.empty() || content.front() == '#') {
			continue;
		}
		const std::size_t separator = content.find('=');
		if (separator == std::string_view::npos) {
			continue;
		}
		const std::string_view key = trim(content.substr(0, separator));
		const std::string_view value = trim(content.substr(separator + 1));
		if (!key.empty() && !value.empty()) {
			data[std::string(key)] = std::string(value);
		}
	}
	if (file.bad()) {
		core::Logger::defaultLogger().error("ConfigParser: Error reading configuration file: " + filepath);
		throw ConfigError("Unable to read configuration file: " + filepath);
	}
	core::Logger::defaultLogger().debug("ConfigParser: Successfully parsed " + std::to_string(data.size()) +
										" entries from " + filepath);
	return data;
}

bool ConfigParser::saveFile(const std::string& filepath, const std::map<std::string, std::string>& data)
{
	std::ofstream file(filepath);
	if (!file.is_open()) {
		core::Logger::defaultLogger().error("ConfigParser: Unable to open configuration file for writing: " + filepath);
		return false;
	}
	for (const auto& [key, value] : data) {
		file << key << " = " << value << '\n';
	}
	file.close();
	if (file.fail()) {
		core::Logger::defaultLogger().error("ConfigParser: Failed to write configuration file: " + filepath);
		return false;
	}
	core::Logger::defaultLogger().debug("ConfigParser: Successfully saved " + std::to_string(data.size()) +
										" entries to " + filepath);
	return true;
}

}
