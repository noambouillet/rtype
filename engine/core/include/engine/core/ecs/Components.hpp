/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Core components
*/

#pragma once

#include <vector>

namespace engine::core {

struct Position {
	float x = 0.0F;
	float y = 0.0F;
};

struct Velocity {
	float x = 0.0F;
	float y = 0.0F;
};

struct Destroy {};

struct PositionSample {
	double time;
	float x;
	float y;
};

struct Interpolated {
	std::vector<PositionSample> samples;
};

}
