/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Audio components
*/

#pragma once

#include <string>

#include "engine/audio/Bus.hpp"

namespace engine::audio {

/// One-shot sound request. Volume is 0-100 before bus volumes, pan is -1 (left) to 1 (right).
struct Sound {
	std::string name;
	float volume = 100.0F;
	Bus bus = Bus::Sfx;
	float pan = 0.0F;
};

}
