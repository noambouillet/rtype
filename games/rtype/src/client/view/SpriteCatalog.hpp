/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite catalog
*/

#pragma once

#include <cstdint>
#include <optional>

#include "engine/core/ecs/Components.hpp"
#include "engine/render/AssetManager.hpp"
#include "game/GameTypes.hpp"

namespace rtype::client {

engine::core::Sprite spriteFor(const engine::render::AssetManager& assets, game::EntityType kind, std::uint8_t variant);
std::optional<engine::core::Animation> animationFor(const engine::render::AssetManager& assets, game::EntityType kind);

}
