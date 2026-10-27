/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Generic key-value configuration file parser
*/

#pragma once

#include <map>
#include <string>
#include <string_view>

namespace engine::core {

class ConfigParser {
public:
	static std::map<std::string, std::string> parseFile(const std::string& filepath);
	static bool saveFile(const std::string& filepath, const std::map<std::string, std::string>& data);

private:
	static std::string_view trim(std::string_view value);
};

}
