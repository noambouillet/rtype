/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** WeaponSystem
*/

#pragma once

#include <optional>
#include <vector>

#include "engine/core/ecs/Components.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"

namespace rtype::ecs {

class WeaponSystem : public engine::core::System {
public:
	engine::core::Signature signature(engine::core::Coordinator& coordinator);
	std::vector<engine::core::Entity> update(engine::core::Coordinator& coordinator, float elapsedTime);

private:
	std::optional<engine::core::Entity> fire(engine::core::Coordinator& coordinator, engine::core::Entity shooter);
	std::optional<engine::core::Entity> fireAtPlayer(engine::core::Coordinator& coordinator, engine::core::Position origin);
};

}
