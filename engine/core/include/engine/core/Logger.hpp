/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Configurable logger
*/

#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <ostream>
#include <string>

namespace engine::core {

class Logger {
public:
	enum class Level : std::uint8_t {
		Debug,
		Info,
		Warn,
		Error,
	};

private:
	void log(Level level, const std::string& message);

	std::ofstream _file;
	std::ostream& _output;
	Level _minLevel;

	static std::shared_ptr<Logger> globalLogger;

public:
	explicit Logger(std::ostream& output);
	explicit Logger(const std::filesystem::path& filename);

	void setVerbose(bool verbose);
	void setLevel(Level level);
	[[nodiscard]] Level level() const;

	void debug(const std::string& message);
	void info(const std::string& message);
	void warn(const std::string& message);
	void error(const std::string& message);

	static void setDefaultLogger(std::shared_ptr<Logger> logger);
	static Logger& defaultLogger();
};

}
