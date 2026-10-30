/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Configurable logger implementation
*/

#include "engine/core/Logger.hpp"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "engine/core/Error.hpp"

namespace engine::core {

std::shared_ptr<Logger> Logger::globalLogger = nullptr;

Logger::Logger(std::ostream& output) : _output(output), _minLevel(Level::Info)
{
}

Logger::Logger(const std::filesystem::path& filename)
	: _file(filename, std::ios::app), _output(_file), _minLevel(Level::Info)
{
	if (!_file) {
		throw LoggerError("Cannot open log file: " + filename.string());
	}
}

void Logger::setVerbose(bool verbose)
{
	_minLevel = verbose ? Level::Debug : Level::Info;
}

void Logger::setLevel(Level level)
{
	_minLevel = level;
}

Logger::Level Logger::level() const
{
	return _minLevel;
}

/// Writes debug messages when debug level is enabled.
void Logger::debug(const std::string& message)
{
	log(Level::Debug, message);
}

/// Writes informational messages when info level is enabled.
void Logger::info(const std::string& message)
{
	log(Level::Info, message);
}

/// Writes warnings when warning level is enabled.
void Logger::warn(const std::string& message)
{
	log(Level::Warn, message);
}

/// Writes errors when error level is enabled.
void Logger::error(const std::string& message)
{
	log(Level::Error, message);
}

/// Writes one entry to this logger instance's output.
void Logger::log(Level level, const std::string& message)
{
	if (level < _minLevel) {
		return;
	}
	const std::time_t now = std::time(nullptr);
	std::tm timeInfo{};

#ifdef _WIN32
	localtime_s(&timeInfo, &now);
#else
	localtime_r(&now, &timeInfo);
#endif

	std::ostringstream oss;
	oss << std::put_time(&timeInfo, "%a %b %d %H:%M:%S %Y");
	const std::string date = oss.str();
	_output << '[' << date << "] ";

	std::string name = "INFO";
	switch (level) {
		case Level::Debug:
			name = "DEBUG";
			break;
		case Level::Info:
			name = "INFO";
			break;
		case Level::Warn:
			name = "WARN";
			break;
		case Level::Error:
			name = "ERROR";
			break;
		default:
			break;
	}
	_output << '[' << name << "] " << message << '\n';
	_output.flush();
}

void Logger::setDefaultLogger(std::shared_ptr<Logger> logger)
{
	globalLogger = std::move(logger);
}

Logger& Logger::defaultLogger()
{
	if (!globalLogger) {
		static auto fallback = std::make_shared<Logger>(std::cout);
		return *fallback;
	}
	return *globalLogger;
}

}
