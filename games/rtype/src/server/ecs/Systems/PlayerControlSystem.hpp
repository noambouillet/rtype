/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** PlayerControlSystem
*/

#pragma once

#include <array>
#include <cstdint>

#include "Coordinator.hpp"
#include "System.hpp"
#include "server/game/GameConfig.hpp"

namespace rtype::ecs {

class PlayerControlSystem : public System {
public:
	using Buttons = std::array<std::uint8_t, config::MAX_PLAYERS>;

	Signature signature(Coordinator& coordinator);
	void update(Coordinator& coordinator, const Buttons& buttons);
};

}
