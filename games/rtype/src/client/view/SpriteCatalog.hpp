/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite catalog
*/

#pragma once

#include <cstdint>
#include <optional>

#include "engine/assets/AssetManager.hpp"
#include "engine/render/Components.hpp"
#include "game/GameTypes.hpp"

namespace rtype::client {

engine::render::Sprite spriteFor(const engine::assets::AssetManager& assets, game::EntityType kind, std::uint8_t variant);
std::optional<engine::render::Animation> animationFor(const engine::assets::AssetManager& assets, game::EntityType kind);

}
