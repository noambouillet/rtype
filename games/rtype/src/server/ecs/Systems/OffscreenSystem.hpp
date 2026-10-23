/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** OffscreenSystem
*/

#pragma once

#include "Coordinator.hpp"
#include "System.hpp"

namespace rtype::ecs {

class OffscreenSystem : public System {
public:
	Signature signature(Coordinator& coordinator);
	void update(Coordinator& coordinator, float worldWidth, float worldHeight, float margin);
};

}
