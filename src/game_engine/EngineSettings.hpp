/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Generic engine settings
*/

#pragma once

#include <map>
#include <string>

namespace rtype::engine {

class EngineSettings {
public:
	void loadFromMap(const std::map<std::string, std::string>& data);
	void saveToMap(std::map<std::string, std::string>& data) const;
	bool screenFlashEnabled() const;

private:
	bool _screenFlashEnabled = true;
};

}
