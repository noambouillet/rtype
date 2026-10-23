/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Generic engine settings
*/

#pragma once

#include <map>
#include <string>

namespace engine::core {

class EngineSettings {
public:
	void loadFromMap(const std::map<std::string, std::string>& data);
	void saveToMap(std::map<std::string, std::string>& data) const;
	bool screenFlashEnabled() const;

private:
	bool _screenFlashEnabled = true;
};

}
