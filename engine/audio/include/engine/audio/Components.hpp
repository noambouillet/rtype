/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Audio components
*/

#pragma once

#include <string>

namespace engine::audio {

struct Sound {
	std::string name;
	float volume = 100.0F;
};

}
