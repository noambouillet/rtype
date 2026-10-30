/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Graphical client entry point
*/

#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "client/app/ClientApp.hpp"
#include "client/app/ClientOptions.hpp"
#include "engine/core/Logger.hpp"

/// Parses command-line arguments and starts the client.
int main(int argc, char** argv)
{
	std::vector<std::string> arguments;
	for (int index = 1; index < argc; index++) {
		arguments.emplace_back(argv[index]);
	}
	const auto options = rtype::parseOptions(arguments);
	if (!options || options->showHelp) {
		std::cout << "Usage: r-type_client [host] [port] [--debug] [--log-file <path> | --log-file=path] [-h]\n";
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

	try {
		return rtype::runClient(*options);
	} catch (const std::exception& error) {
		logger->error(std::string("Client error: ") + error.what());
		return 1;
	}
}
