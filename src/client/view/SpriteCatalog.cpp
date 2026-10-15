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
ecs::Sprite spriteFor(game::EntityType kind, std::uint8_t variant)
{
	switch (kind) {
		case game::EntityType::Player: {
			const std::size_t index = variant < sprites::PLAYER_SHIPS.size() ? variant : 0;
			return {std::string(sprites::PLAYER_SHEET), sprites::PLAYER_SHIPS[index], 16.0F, 8.0F, 3.0F, 0.0F, 0};
		}
		case game::EntityType::PlayerProjectile:
			return {std::string(sprites::SHOT_SHEET), sprites::PLAYER_SHOT_FRAMES[0], 8.0F, 6.0F, 2.0F, 0.0F, 0};
		case game::EntityType::EnemyProjectile:
			if (static_cast<game::ShotKind>(variant) == game::ShotKind::Beam) {
				return {std::string(sprites::SCANT_SHEET), sprites::SCANT_BEAM, 27.0F, 8.0F, 2.0F, 0.0F, 0};
			}
			return {std::string(sprites::ENEMY_SHOT_SHEET), sprites::ENEMY_SHOT, 3.5F, 3.0F, 2.0F, 0.0F, 0};
		case game::EntityType::Enemy:
			switch (static_cast<game::EnemyKind>(variant)) {
				case game::EnemyKind::Bug:
					return {std::string(sprites::BUG_SHEET), sprites::ENEMY_BUG, 16.5F, 17.0F, 3.0F, 0.0F, 0};
				case game::EnemyKind::Scant:
					return {std::string(sprites::SCANT_SHEET), sprites::ENEMY_SCANT, 28.0F, 27.0F, 3.0F, 0.0F, 0};
				case game::EnemyKind::Bink:
					return {std::string(sprites::BINK_SHEET), sprites::ENEMY_BINK, 16.0F, 14.5F, 3.0F, 0.0F, 0};
				case game::EnemyKind::Patapata:
					return {std::string(sprites::ENEMY_SHEET), sprites::ENEMY_PATAPATA, 16.5F, 18.0F, 3.0F, 0.0F, 0};
			}
	}
	return {};
}

/// Supplies the looping projectile frames; other entities are static.
std::optional<ecs::Animation> animationFor(game::EntityType kind)
{
	if (kind != game::EntityType::PlayerProjectile) {
		return std::nullopt;
	}
	return ecs::Animation{{sprites::PLAYER_SHOT_FRAMES.begin(), sprites::PLAYER_SHOT_FRAMES.end()}, 0.08F, 0.0F, true};
}

}
