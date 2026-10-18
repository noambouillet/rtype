/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Audio backend interface
*/

#pragma once

#include <string_view>

namespace rtype::audio {

class IAudioBackend {
public:
	virtual ~IAudioBackend() = default;
	virtual void playMusic(std::string_view name, float volume) = 0;
	virtual void stopMusic() = 0;
	virtual void playSound(std::string_view name, float volume) = 0;
};

}
