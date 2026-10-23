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

#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"
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

class ReplicationSystem : public engine::core::System {
public:
	engine::core::Signature signature(engine::core::Coordinator& coordinator);
	std::optional<ReplicatedState> capture(engine::core::Coordinator& coordinator, engine::core::Entity entity) const;
	std::vector<ReplicatedState> collect(engine::core::Coordinator& coordinator);
};

}
