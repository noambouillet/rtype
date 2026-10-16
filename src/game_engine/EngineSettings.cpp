/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Generic engine settings
*/

#include "EngineSettings.hpp"

namespace rtype::engine {

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
