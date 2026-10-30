/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** InterpolationSystem
*/

#pragma once

#include <cstddef>

#include "engine/core/ecs/Components.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"

namespace engine::core {

class InterpolationSystem : public System {
public:
	static constexpr std::size_t MAX_SAMPLES = 16;

	static Signature signature(Coordinator& coordinator);
	static void addSample(Interpolated& interpolated, const PositionSample& sample);
	void update(Coordinator& coordinator, double renderTime);
};

}
