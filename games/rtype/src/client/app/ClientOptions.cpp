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

/// Applies one command-line argument to the client settings.
static bool applyArgument(ClientOptions& options, const std::string& argument, int& position)
{
	if (argument.starts_with("--log-file=") && argument.size() > 11) {
		options.logFile = argument.substr(11);
		return true;
	}
	if (position == 0 && parsePort(argument, options.port)) {
		position = 2;
		return true;
	}
	if (position == 0 && !argument.starts_with('-')) {
		options.host = argument;
		position = 1;
		return true;
	}
	if (position == 1 && parsePort(argument, options.port)) {
		position = 2;
		return true;
	}
	return false;
}

/// Reads an optional hostname, port, and log file from string arguments.
std::optional<ClientOptions> parseOptions(const std::vector<std::string>& arguments)
{
	ClientOptions options;
	options.host = "localhost";
	options.port = 6767;
	options.logFile = "";
	options.showHelp = false;
	int position = 0;
	for (const std::string& argument : arguments) {
		if (argument == "-h" || argument == "--help") {
			options.showHelp = true;
			return options;
		}
		if (!applyArgument(options, argument, position)) {
			return std::nullopt;
		}
	}
	return options;
}

}
