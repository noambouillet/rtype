/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite catalog
*/

#include "SpriteCatalog.hpp"

#include <string>

#include "engine/render/AssetManager.hpp"
#include "game/GameTypes.hpp"

namespace rtype::client {

/// Selects the image and geometry used to present a gameplay entity.
engine::render::Sprite spriteFor(const engine::render::AssetManager& assets, game::EntityType kind, std::uint8_t variant)
{
	switch (kind) {
		case game::EntityType::Player: {
			const std::string spriteName = "player_ship_" + std::to_string(variant < 4 ? variant : 0);
			const auto& desc = assets.getSprite(spriteName);
			return {.texture = desc.texture, .region = desc.rect, .originX = desc.originX, .originY = desc.originY, .scale = 3.0F, .rotation = 0.0F, .layer = 0};
		}
		case game::EntityType::PlayerProjectile: {
			const auto& desc = assets.getSprite("player_shot");
			return {.texture = desc.texture, .region = desc.rect, .originX = desc.originX, .originY = desc.originY, .scale = 2.0F, .rotation = 0.0F, .layer = 0};
		}
		case game::EntityType::EnemyProjectile: {
			if (static_cast<game::ShotKind>(variant) == game::ShotKind::Beam) {
				const auto& desc = assets.getSprite("scant_beam");
				return {.texture = desc.texture, .region = desc.rect, .originX = desc.originX, .originY = desc.originY, .scale = 2.0F, .rotation = 0.0F, .layer = 0};
			}
			const auto& desc = assets.getSprite("enemy_shot");
			return {.texture = desc.texture, .region = desc.rect, .originX = desc.originX, .originY = desc.originY, .scale = 2.0F, .rotation = 0.0F, .layer = 0};
		}
		case game::EntityType::Enemy: {
			switch (static_cast<game::EnemyKind>(variant)) {
				case game::EnemyKind::Bug: {
					const auto& desc = assets.getSprite("enemy_bug");
					return {.texture = desc.texture, .region = desc.rect, .originX = desc.originX, .originY = desc.originY, .scale = 3.0F, .rotation = 0.0F, .layer = 0};
				}
				case game::EnemyKind::Scant: {
					const auto& desc = assets.getSprite("enemy_scant");
					return {.texture = desc.texture, .region = desc.rect, .originX = desc.originX, .originY = desc.originY, .scale = 3.0F, .rotation = 0.0F, .layer = 0};
				}
				case game::EnemyKind::Bink: {
					const auto& desc = assets.getSprite("enemy_bink");
					return {.texture = desc.texture, .region = desc.rect, .originX = desc.originX, .originY = desc.originY, .scale = 3.0F, .rotation = 0.0F, .layer = 0};
				}
				case game::EnemyKind::Patapata: {
					const auto& desc = assets.getSprite("enemy_patapata");
					return {.texture = desc.texture, .region = desc.rect, .originX = desc.originX, .originY = desc.originY, .scale = 3.0F, .rotation = 0.0F, .layer = 0};
				}
			}
		}
	}
	return {};
}

/// Supplies the looping projectile frames; other entities are static.
std::optional<engine::render::Animation> animationFor(const engine::render::AssetManager& assets, game::EntityType kind)
{
	if (kind != game::EntityType::PlayerProjectile) {
		return std::nullopt;
	}
	const auto& desc = assets.getAnimation("player_shot");
	return engine::render::Animation{.texture = desc.texture, .frames = desc.frames, .frameSeconds = desc.frameSeconds, .elapsed = 0.0F, .loop = desc.loop};
}

}
