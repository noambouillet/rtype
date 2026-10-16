/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Generic key-value configuration file parser
*/

#pragma once

#include <map>
#include <string>

namespace rtype::engine {

class ConfigParser {
public:
	static std::map<std::string, std::string> parseFile(const std::string& filepath);
	static bool saveFile(const std::string& filepath, const std::map<std::string, std::string>& data);
};

}
