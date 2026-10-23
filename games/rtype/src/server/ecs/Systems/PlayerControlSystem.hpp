/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** PlayerControlSystem
*/

#pragma once

#include <array>
#include <cstdint>

#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"
#include "server/game/GameConfig.hpp"

namespace rtype::ecs {

class PlayerControlSystem : public engine::core::System {
public:
	using Buttons = std::array<std::uint8_t, config::MAX_PLAYERS>;

	engine::core::Signature signature(engine::core::Coordinator& coordinator);
	void update(engine::core::Coordinator& coordinator, const Buttons& buttons);
};

}
