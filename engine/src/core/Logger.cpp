/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Configurable logger implementation
*/

#include "Logger.hpp"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

#include "RTypeError.hpp"

namespace rtype {

Logger::Logger(std::ostream& output) : _output(output), _verbose(false)
{
}

Logger::Logger(const std::filesystem::path& filename) : _file(filename, std::ios::app), _output(_file), _verbose(false)
{
	if (!_file) {
		throw LoggerError("Cannot open log file: " + filename.string());
	}
}

void Logger::setVerbose(bool verbose)
{
	_verbose = verbose;
}

/// Writes a timestamped message to the configured stream.
void Logger::write(const std::string& message)
{
	log(Level::Plain, message);
}

/// Writes informational messages when verbose output is enabled.
void Logger::info(const std::string& message)
{
	log(Level::Info, message);
}

/// Writes warnings when verbose output is enabled.
void Logger::warn(const std::string& message)
{
	log(Level::Warn, message);
}

/// Writes errors regardless of the verbose setting.
void Logger::error(const std::string& message)
{
	log(Level::Error, message);
}

/// Writes one entry to this logger instance's output.
void Logger::log(Level level, const std::string& message)
{
	if ((level == Level::Info || level == Level::Warn) && !_verbose) {
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
	if (level != Level::Plain) {
		std::string name = "ERROR";
		if (level == Level::Info) {
			name = "INFO";
		} else if (level == Level::Warn) {
			name = "WARN";
		}
		_output << '[' << name << "] ";
	}
	_output << message << '\n';
	_output.flush();
}

}
