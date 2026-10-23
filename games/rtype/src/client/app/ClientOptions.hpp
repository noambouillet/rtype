/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client command-line options
*/

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace rtype {

struct ClientOptions {
	std::string host;
	std::uint16_t port;
	std::string logFile;
	bool showHelp;
};

std::optional<ClientOptions> parseOptions(const std::vector<std::string>& arguments);

}
