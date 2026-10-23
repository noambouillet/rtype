/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Configurable logger
*/

#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ostream>
#include <string>

namespace rtype {

class Logger {
private:
	enum class Level : std::uint8_t {
		Plain,
		Info,
		Warn,
		Error
	};
	void log(Level level, const std::string& message);

	std::ofstream _file;
	std::ostream& _output;
	bool _verbose;

public:
	explicit Logger(std::ostream& output);
	explicit Logger(const std::filesystem::path& filename);

	void setVerbose(bool verbose);
	void write(const std::string& message);
	void info(const std::string& message);
	void warn(const std::string& message);
	void error(const std::string& message);
};

}
