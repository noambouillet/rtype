/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Audio backend interface
*/

#pragma once

#include <chrono>
#include <string_view>

#include "engine/audio/Bus.hpp"

namespace engine::audio {

using Seconds = std::chrono::duration<float>;

class IAudioBackend {
public:
	virtual ~IAudioBackend() = default;
	virtual void playMusic(std::string_view name, float volume, Seconds fade) = 0;
	virtual void stopMusic(Seconds fade) = 0;
	virtual void playSound(std::string_view name, float volume, Bus bus, float pan) = 0;
	virtual void setBusVolume(Bus bus, float volume) = 0;
	[[nodiscard]] virtual float busVolume(Bus bus) const = 0;
	virtual void update(float deltaTime) = 0;
};

}
