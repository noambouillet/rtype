/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Generic engine settings
*/

#include "engine/core/EngineSettings.hpp"

namespace engine::core {

void EngineSettings::loadFromMap(const std::map<std::string, std::string>& data)
{
	const auto screenFlash = data.find("screen_flash");
	if (screenFlash == data.end())
		return;
	if (screenFlash->second == "true") {
		_screenFlashEnabled = true;
	} else if (screenFlash->second == "false") {
		_screenFlashEnabled = false;
	}
}

void EngineSettings::saveToMap(std::map<std::string, std::string>& data) const
{
	data["screen_flash"] = _screenFlashEnabled ? "true" : "false";
}

bool EngineSettings::screenFlashEnabled() const
{
	return _screenFlashEnabled;
}

}
