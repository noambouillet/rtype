/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Graphical client entry point
*/

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "client/app/ClientApp.hpp"
#include "client/app/ClientOptions.hpp"

/// Parses command-line arguments and starts the client.
int main(int argc, char** argv)
{
	std::vector<std::string> arguments;
	for (int index = 1; index < argc; index++) {
		arguments.emplace_back(argv[index]);
	}
	const auto options = rtype::parseOptions(arguments);
	if (!options || options->showHelp) {
		std::cout << "Usage: r-type_client [host] [port] [--log-file=path] [-h]\n";
		return options ? 0 : 2;
	}
	try {
		return rtype::runClient(*options);
	} catch (const std::exception& error) {
		std::cerr << "Client error: " << error.what() << '\n';
		return 1;
	}
}
