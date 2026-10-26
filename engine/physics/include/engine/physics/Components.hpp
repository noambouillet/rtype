/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Physics components
*/

#pragma once

#include <cstdint>

namespace engine::physics {

struct Collider {
	float width = 0.0F;
	float height = 0.0F;
	float offsetX = 0.0F;
	float offsetY = 0.0F;
	std::uint32_t tag = 0;
	std::uint32_t mask = 0;
};

}
