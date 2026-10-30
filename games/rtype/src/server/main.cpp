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
#include "engine/core/Logger.hpp"
#include "engine/net/UdpServer.hpp"
#include "utils/network/Protocol.hpp"

namespace rtype {

struct ServerOptions {
	std::uint16_t port = 6767;
	std::string logFile;
	bool isDebugMode = false;
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

/// Applies a single argument or flag to the server options, updating the index and positional counter.
static bool applyArgument(int argc, char** argv, int& index,
	ServerOptions& options, int& positionalIndex)
{
	const std::string_view argument(argv[index]);

	if (argument == "-h" || argument == "--help") {
		options.showHelp = true;
		return true;
	}
	if (argument == "--debug") {
		options.isDebugMode = true;
		return true;
	}
	if (argument.starts_with("--log-file=") && argument.size() > 11) {
		options.logFile = std::string(argument.substr(11));
		return true;
	}
	if (argument == "--log-file") {
		if (index + 1 >= argc) {
			return false;
		}
		options.logFile = argv[++index];
		return true;
	}
	if (positionalIndex == 0 && parsePort(argument, options.port)) {
		positionalIndex = 1;
		return true;
	}
	return false;
}

/// Reads the optional UDP port, debug flag, and log file from the command line.
static std::optional<ServerOptions> parseOptions(int argc, char** argv)
{
	ServerOptions options;
	int positionalIndex = 0;

	for (int index = 1; index < argc; ++index) {
		if (!applyArgument(argc, argv, index, options, positionalIndex)) {
			return std::nullopt;
		}
		if (options.showHelp) {
			return options;
		}
	}
	return options;
}

/// Starts the server with the selected log destination.
static int runServer(const ServerOptions& options, engine::core::Logger& logger)
{
	std::signal(SIGINT, stopServer);
	std::signal(SIGTERM, stopServer);

	try {
		logger.debug("Initializing server network subsystem on UDP port " + std::to_string(options.port));
		engine::net::UdpServer transport({.port = options.port, .maxDatagramSize = network::MAX_DATAGRAM_SIZE});
		logger.debug("Server network subsystem initialized successfully");

		logger.debug("Initializing GameServer core...");
		GameServer server(transport, logger);
		logger.info("GameServer ready and listening on UDP port " + std::to_string(options.port));

		server.run(running);
		logger.info("Shutting down server subsystems...");
		logger.info("Server teardown completed. Goodbye...");
	} catch (const std::exception& error) {
		logger.error(std::string("Server error: ") + error.what());
		return 1;
	}
	return 0;
}

}

int main(int argc, char** argv)
{
	const auto options = rtype::parseOptions(argc, argv);
	if (!options || options->showHelp) {
		std::cout << "Usage: r-type_server [udp-port] [--debug] [--log-file <path> | --log-file=path] [-h]\n";
		return options ? 0 : 2;
	}

	std::shared_ptr<engine::core::Logger> logger;
	if (options->logFile.empty()) {
		logger = std::make_shared<engine::core::Logger>(std::cout);
	} else {
		logger = std::make_shared<engine::core::Logger>(std::filesystem::path(options->logFile));
	}
	if (options->isDebugMode) {
		logger->setVerbose(true);
	}
	engine::core::Logger::setDefaultLogger(logger);

	return rtype::runServer(*options, *logger);
}
