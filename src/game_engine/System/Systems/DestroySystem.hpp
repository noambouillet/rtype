/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** DestroySystem
*/

#pragma once

#include "Components.hpp"
#include "Coordinator.hpp"
#include "System.hpp"

namespace rtype::ecs {

class DestroySystem : public System {
public:
	Signature signature(Coordinator& coordinator);
	void update(Coordinator& coordinator);
};

}
