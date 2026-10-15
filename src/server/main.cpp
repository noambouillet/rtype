/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Entrypoint of the R-Type server
*/

#include <charconv>
#include <csignal>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

#include "GameServer.hpp"
#include "network/UdpServer.hpp"
#include "utils/Logger.hpp"

namespace rtype {

struct ServerOptions {
	std::uint16_t port = 6767;
	std::string logFile;
	bool showHelp = false;
};

static volatile std::sig_atomic_t running = 1;

static void stopServer([[maybe_unused]] int signal)
{
	running = 0;
}

/// Parses a decimal UDP port in the inclusive range 1 to 65535.
static bool parsePort(std::string_view text, std::uint16_t& port)
{
	unsigned int value = 0;

	const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
	if (error != std::errc{} || end != text.data() + text.size() || value == 0 || value > 65535) {
		return false;
	}
	port = static_cast<std::uint16_t>(value);
	return true;
}

/// Reads the optional UDP port and log file from the command line.
static std::optional<ServerOptions> parseOptions(int argc, char** argv)
{
	ServerOptions options;
	bool hasPort = false;

	for (int index = 1; index < argc; index++) {
		const std::string_view argument(argv[index]);
		if (argument == "-h" || argument == "--help") {
			options.showHelp = true;
			return options;
		}
		if (argument.starts_with("--log-file=") && argument.size() > 11) {
			options.logFile = argument.substr(11);
			continue;
		}
		if (!hasPort && parsePort(argument, options.port)) {
			hasPort = true;
			continue;
		}
		return std::nullopt;
	}
	return options;
}

/// Starts the server with the selected log destination.
static int runServer(const ServerOptions& options)
{
	std::signal(SIGINT, stopServer);
	std::signal(SIGTERM, stopServer);
	std::unique_ptr<Logger> logger;

	try {
		if (options.logFile.empty()) {
			logger = std::make_unique<Logger>(std::cout);
		} else {
			logger = std::make_unique<Logger>(std::filesystem::path(options.logFile));
		}
		logger->setVerbose(true);
		network::UdpServer transport(options.port);
		GameServer server(transport);
		logger->info("R-Type server listening on UDP port " + std::to_string(options.port));
		server.run(running);
		logger->info("Goodbye...");
	} catch (const std::exception& error) {
		if (logger) {
			logger->error(std::string("Server error: ") + error.what());
		} else {
			std::cerr << "Server error: " << error.what() << '\n';
		}
		return 1;
	}
	return 0;
}

}

int main(int argc, char** argv)
{
	const auto options = rtype::parseOptions(argc, argv);
	if (!options || options->showHelp) {
		std::ostream& output = options ? std::cout : std::cerr;
		output << "Usage: r-type_server [udp-port] [--log-file=path] [-h]\n";
		return options ? 0 : 2;
	}
	return rtype::runServer(*options);
}
