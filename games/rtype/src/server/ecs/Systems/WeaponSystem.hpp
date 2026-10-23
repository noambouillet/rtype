/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** WeaponSystem
*/

#pragma once

#include <optional>
#include <vector>

#include "Components.hpp"
#include "Coordinator.hpp"
#include "System.hpp"

namespace rtype::ecs {

class WeaponSystem : public System {
public:
	Signature signature(Coordinator& coordinator);
	std::vector<Entity> update(Coordinator& coordinator, float elapsedTime);

private:
	std::optional<Entity> fire(Coordinator& coordinator, Entity shooter);
	std::optional<Entity> fireAtPlayer(Coordinator& coordinator, Position origin);
};

}
