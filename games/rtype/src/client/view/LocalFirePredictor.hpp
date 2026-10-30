/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Timing for the local firing effect
*/

#pragma once

#include <chrono>

namespace rtype {

class LocalFirePredictor {
public:
	using TimePoint = std::chrono::steady_clock::time_point;

	LocalFirePredictor();
	bool update(bool firing, bool active, TimePoint now);

private:
	TimePoint _nextShot;
};

}
