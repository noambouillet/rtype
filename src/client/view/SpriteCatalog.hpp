/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite catalog
*/

#pragma once

#include <cstdint>
#include <optional>

#include "Components.hpp"
#include "game/GameTypes.hpp"

namespace rtype::client {

ecs::Sprite spriteFor(game::EntityType kind, std::uint8_t variant);
std::optional<ecs::Animation> animationFor(game::EntityType kind);

}
