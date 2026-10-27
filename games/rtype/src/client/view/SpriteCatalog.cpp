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
engine::core::Sprite spriteFor(const engine::render::AssetManager& assets, game::EntityType kind, std::uint8_t variant)
{
	switch (kind) {
		case game::EntityType::Player: {
			const std::string spriteName = "player_ship_" + std::to_string(variant < 4 ? variant : 0);
			const auto& desc = assets.getSprite(spriteName);
			return {desc.texture, desc.rect, desc.originX, desc.originY, 3.0F, 0.0F, 0};
		}
		case game::EntityType::PlayerProjectile: {
			const auto& desc = assets.getSprite("player_shot");
			return {desc.texture, desc.rect, desc.originX, desc.originY, 2.0F, 0.0F, 0};
		}
		case game::EntityType::EnemyProjectile: {
			if (static_cast<game::ShotKind>(variant) == game::ShotKind::Beam) {
				const auto& desc = assets.getSprite("scant_beam");
				return {desc.texture, desc.rect, desc.originX, desc.originY, 2.0F, 0.0F, 0};
			}
			const auto& desc = assets.getSprite("enemy_shot");
			return {desc.texture, desc.rect, desc.originX, desc.originY, 2.0F, 0.0F, 0};
		}
		case game::EntityType::Enemy: {
			switch (static_cast<game::EnemyKind>(variant)) {
				case game::EnemyKind::Bug: {
					const auto& desc = assets.getSprite("enemy_bug");
					return {desc.texture, desc.rect, desc.originX, desc.originY, 3.0F, 0.0F, 0};
				}
				case game::EnemyKind::Scant: {
					const auto& desc = assets.getSprite("enemy_scant");
					return {desc.texture, desc.rect, desc.originX, desc.originY, 3.0F, 0.0F, 0};
				}
				case game::EnemyKind::Bink: {
					const auto& desc = assets.getSprite("enemy_bink");
					return {desc.texture, desc.rect, desc.originX, desc.originY, 3.0F, 0.0F, 0};
				}
				case game::EnemyKind::Patapata: {
					const auto& desc = assets.getSprite("enemy_patapata");
					return {desc.texture, desc.rect, desc.originX, desc.originY, 3.0F, 0.0F, 0};
				}
			}
		}
	}
	return {};
}

/// Supplies the looping projectile frames; other entities are static.
std::optional<engine::core::Animation> animationFor(const engine::render::AssetManager& assets, game::EntityType kind)
{
	if (kind != game::EntityType::PlayerProjectile) {
		return std::nullopt;
	}
	const auto& desc = assets.getAnimation("player_shot");
	return engine::core::Animation{desc.texture, desc.frames, desc.frameSeconds, 0.0F, desc.loop};
}

}
