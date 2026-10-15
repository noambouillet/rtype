/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ReplicationSystem
*/

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "Coordinator.hpp"
#include "System.hpp"
#include "game/GameTypes.hpp"

namespace rtype::ecs {

struct ReplicatedState {
	std::uint32_t id;
	game::EntityType type;
	std::uint8_t owner;
	std::uint8_t variant;
	std::uint8_t health;
	float x;
	float y;
};

class ReplicationSystem : public System {
public:
	Signature signature(Coordinator& coordinator);
	std::optional<ReplicatedState> capture(Coordinator& coordinator, Entity entity) const;
	std::vector<ReplicatedState> collect(Coordinator& coordinator);
};

}
