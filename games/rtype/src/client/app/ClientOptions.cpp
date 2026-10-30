/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client command-line parsing
*/

#include "ClientOptions.hpp"

#include <charconv>
#include <system_error>

namespace rtype {

/// Parses a decimal port in the inclusive range 1 to 65535.
static bool parsePort(const std::string& value, std::uint16_t& port)
{
	unsigned int parsed = 0;
	const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), parsed);
	if (error != std::errc{} || end != value.data() + value.size() || parsed == 0 || parsed > 65535) {
		return false;
	}
	port = static_cast<std::uint16_t>(parsed);
	return true;
}

/// Applies a single argument or flag to the options, updating the index and positional counter.
static bool applyArgument(const std::vector<std::string>& arguments, std::size_t& index,
	ClientOptions& options, int& positionalIndex)
{
	const std::string& argument = arguments[index];

	if (argument == "-h" || argument == "--help") {
		options.showHelp = true;
		return true;
	}
	if (argument == "--debug") {
		options.isDebugMode = true;
		return true;
	}
	if (argument.starts_with("--log-file=") && argument.size() > 11) {
		options.logFile = argument.substr(11);
		return true;
	}
	if (argument == "--log-file") {
		if (index + 1 >= arguments.size()) {
			return false;
		}
		options.logFile = arguments[++index];
		return true;
	}
	if (positionalIndex == 0 && parsePort(argument, options.port)) {
		positionalIndex = 2;
		return true;
	}
	if (positionalIndex == 0 && !argument.starts_with('-')) {
		options.host = argument;
		positionalIndex = 1;
		return true;
	}
	if (positionalIndex == 1 && parsePort(argument, options.port)) {
		positionalIndex = 2;
		return true;
	}
	return false;
}

/// Reads an optional hostname, port, debug flag, and log file from string arguments.
std::optional<ClientOptions> parseOptions(const std::vector<std::string>& arguments)
{
	ClientOptions options;
	options.host = "localhost";
	options.port = 6767;
	options.logFile = "";
	options.showHelp = false;
	options.isDebugMode = false;
	int positionalIndex = 0;

	for (std::size_t index = 0; index < arguments.size(); ++index) {
		if (!applyArgument(arguments, index, options, positionalIndex)) {
			return std::nullopt;
		}
		if (options.showHelp) {
			return options;
		}
	}
	return options;
}

}
