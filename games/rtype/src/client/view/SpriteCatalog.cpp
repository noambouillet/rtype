/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite catalog
*/

#include "SpriteCatalog.hpp"

#include "client/view/SpriteRegions.hpp"
#include "game/GameTypes.hpp"

namespace rtype::client {

/// Selects the image and geometry used to present a gameplay entity.
engine::render::Sprite spriteFor(game::EntityType kind, std::uint8_t variant)
{
	switch (kind) {
		case game::EntityType::Player: {
			const std::size_t index = variant < sprites::PLAYER_SHIPS.size() ? variant : 0;
			return {.textureName = std::string(sprites::PLAYER_SHEET), .region = sprites::PLAYER_SHIPS[index], .originX = 16.0F, .originY = 8.0F, .scale = 3.0F, .rotation = 0.0F, .layer = 0};
		}
		case game::EntityType::PlayerProjectile:
			return {.textureName = std::string(sprites::SHOT_SHEET), .region = sprites::PLAYER_SHOT_FRAMES[0], .originX = 8.0F, .originY = 6.0F, .scale = 2.0F, .rotation = 0.0F, .layer = 0};
		case game::EntityType::EnemyProjectile:
			if (static_cast<game::ShotKind>(variant) == game::ShotKind::Beam) {
				return {.textureName = std::string(sprites::SCANT_SHEET), .region = sprites::SCANT_BEAM, .originX = 27.0F, .originY = 8.0F, .scale = 2.0F, .rotation = 0.0F, .layer = 0};
			}
			return {.textureName = std::string(sprites::ENEMY_SHOT_SHEET), .region = sprites::ENEMY_SHOT, .originX = 3.5F, .originY = 3.0F, .scale = 2.0F, .rotation = 0.0F, .layer = 0};
		case game::EntityType::Enemy:
			switch (static_cast<game::EnemyKind>(variant)) {
				case game::EnemyKind::Bug:
					return {.textureName = std::string(sprites::BUG_SHEET), .region = sprites::ENEMY_BUG, .originX = 16.5F, .originY = 17.0F, .scale = 3.0F, .rotation = 0.0F, .layer = 0};
				case game::EnemyKind::Scant:
					return {.textureName = std::string(sprites::SCANT_SHEET), .region = sprites::ENEMY_SCANT, .originX = 28.0F, .originY = 27.0F, .scale = 3.0F, .rotation = 0.0F, .layer = 0};
				case game::EnemyKind::Bink:
					return {.textureName = std::string(sprites::BINK_SHEET), .region = sprites::ENEMY_BINK, .originX = 16.0F, .originY = 14.5F, .scale = 3.0F, .rotation = 0.0F, .layer = 0};
				case game::EnemyKind::Patapata:
					return {.textureName = std::string(sprites::ENEMY_SHEET), .region = sprites::ENEMY_PATAPATA, .originX = 16.5F, .originY = 18.0F, .scale = 3.0F, .rotation = 0.0F, .layer = 0};
			}
	}
	return {};
}

/// Supplies the looping projectile frames; other entities are static.
std::optional<engine::render::Animation> animationFor(game::EntityType kind)
{
	if (kind != game::EntityType::PlayerProjectile) {
		return std::nullopt;
	}
	return engine::render::Animation{.frames = {sprites::PLAYER_SHOT_FRAMES.begin(), sprites::PLAYER_SHOT_FRAMES.end()}, .frameSeconds = 0.08F, .elapsed = 0.0F, .loop = true};
}

}
