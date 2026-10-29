/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Effect presentation system
*/

#include "EffectSystem.hpp"

#include <algorithm>
#include <string>
#include <variant>

#include "engine/assets/AssetManager.hpp"
#include "engine/core/ecs/Components.hpp"
#include "engine/render/Components.hpp"
#include "game/GameTypes.hpp"

namespace rtype::client {

static std::string_view animationNameFor(EffectKind kind)
{
	switch (kind) {
		case EffectKind::ShotBurst:
			return "shot_burst";
		case EffectKind::HitSpark:
			return "hit_spark";
		case EffectKind::MobExplosion:
			return "mob_explosion";
		case EffectKind::PlayerExplosion:
			return "player_explosion";
	}
	return "";
}

EffectSystem::EffectSystem() : _firePredictor{}, _screenFlash(true), _assets(nullptr)
{
}

void EffectSystem::setAssetManager(const engine::assets::AssetManager& assets)
{
	_assets = &assets;
}

/// Enables or disables explosion effects, which flash the screen (accessibility setting).
void EffectSystem::setScreenFlash(bool enabled)
{
	_screenFlash = enabled;
}

engine::core::Signature EffectSystem::signature(engine::core::Coordinator& coordinator)
{
	engine::core::Signature signature;
	signature.set(coordinator.getComponentType<Effect>());
	return signature;
}

/// Adds one bounded effect entity and keeps repeated hits from stacking.
void EffectSystem::add(engine::core::Coordinator& coordinator, EffectKind kind, engine::core::Position position,
	std::optional<std::uint32_t> targetId, std::optional<std::uint8_t> followPlayer)
{
	if (targetId) {
		for (engine::core::Entity existing : entities) {
			const Effect& effect = coordinator.getComponent<Effect>(existing);
			if (effect.targetId == targetId && effect.kind == kind) {
				coordinator.getComponent<engine::core::Position>(existing) = position;
				return;
			}
		}
	}
	if (!_screenFlash && (kind == EffectKind::MobExplosion || kind == EffectKind::PlayerExplosion)) {
		return;
	}
	if (entities.size() >= MAX_EFFECTS) {
		coordinator.destroyEntity(*entities.begin());
	}
	if (!_assets) {
		return;
	}
	const auto& animDesc = _assets->getAnimation(std::string(animationNameFor(kind)));
	if (animDesc.frames.empty()) {
		return;
	}
	const engine::render::Rect first = animDesc.frames.front();
	const engine::core::Entity entity = coordinator.createEntity();
	coordinator.addComponent(entity, position);
	coordinator.addComponent(entity, engine::render::Sprite{.texture = animDesc.texture, .region = first, .originX = static_cast<float>(first.size.x) / 2.0F, .originY = static_cast<float>(first.size.y) / 2.0F, .scale = 2.0F, .rotation = 0.0F, .layer = 10});
	coordinator.addComponent(entity, engine::render::Animation{.texture = animDesc.texture, .frames = animDesc.frames, .frameSeconds = animDesc.frameSeconds, .elapsed = 0.0F, .loop = animDesc.loop});
	coordinator.addComponent(entity, Effect{.kind = kind, .remaining = animDesc.frameSeconds * static_cast<float>(animDesc.frames.size()), .targetId = targetId, .followPlayer = followPlayer});
}

/// Removes a surviving-hit cue when that entity is destroyed.
void EffectSystem::removeHitSpark(engine::core::Coordinator& coordinator, std::uint32_t targetId)
{
	std::vector<engine::core::Entity> removed;
	for (engine::core::Entity entity : entities) {
		const Effect& effect = coordinator.getComponent<Effect>(entity);
		if (effect.kind == EffectKind::HitSpark && effect.targetId == targetId) {
			removed.push_back(entity);
		}
	}
	for (engine::core::Entity entity : removed) {
		coordinator.destroyEntity(entity);
	}
}

void EffectSystem::handle(engine::core::Coordinator& coordinator, [[maybe_unused]] const ClientNetworkSystem& network,
	const SpawnEvent& event, std::optional<std::uint8_t> localPlayerId)
{
	if (event.kind == game::EntityType::PlayerProjectile && (!localPlayerId || event.owner != *localPlayerId)) {
		add(coordinator, EffectKind::ShotBurst, {.x = event.x, .y = event.y}, event.id, event.owner);
	}
}

void EffectSystem::handle(engine::core::Coordinator& coordinator, [[maybe_unused]] const ClientNetworkSystem& network,
	const DestroyEvent& event)
{
	removeHitSpark(coordinator, event.id);
	if (event.kind == game::EntityType::Enemy && destroyedOnscreen(event)) {
		add(coordinator, EffectKind::MobExplosion, {.x = event.x, .y = event.y}, event.id);
	}
}

void EffectSystem::handle(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network, const DamageEvent& event)
{
	if (const auto entity = network.find(event.id)) {
		add(coordinator, EffectKind::HitSpark, coordinator.getComponent<engine::core::Position>(*entity), event.id);
	}
}

void EffectSystem::handle(engine::core::Coordinator& coordinator, [[maybe_unused]] const ClientNetworkSystem& network,
	const DeathEvent& event)
{
	removeHitSpark(coordinator, event.id);
	add(coordinator, EffectKind::PlayerExplosion, {.x = event.x, .y = event.y}, event.id);
}

/// Converts game notices into effect entities before rendering the frame.
void EffectSystem::consume(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network,
	const std::vector<ViewEvent>& events, std::optional<std::uint8_t> localPlayerId)
{
	for (const ViewEvent& event : events) {
		if (const auto* spawn = std::get_if<SpawnEvent>(&event)) {
			handle(coordinator, network, *spawn, localPlayerId);
		} else if (const auto* destroy = std::get_if<DestroyEvent>(&event)) {
			handle(coordinator, network, *destroy);
		} else if (const auto* damage = std::get_if<DamageEvent>(&event)) {
			handle(coordinator, network, *damage);
		} else {
			handle(coordinator, network, std::get<DeathEvent>(event));
		}
	}
}

/// Starts the local weapon burst immediately while the server owns the shot entity.
bool EffectSystem::predictLocalFire(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network,
	std::uint8_t buttons, std::optional<std::uint8_t> localPlayerId)
{
	std::optional<engine::core::Position> position;
	if (localPlayerId && (network.aliveMask() & (1 << *localPlayerId)) != 0) {
		position = network.playerPosition(coordinator, *localPlayerId);
	}
	const auto now = LocalFirePredictor::TimePoint::clock::now();
	if (_firePredictor.update((buttons & rtype::game::BUTTON_FIRE) != 0, position.has_value(), now) && position) {
		position->x += PLAYER_WEAPON_OFFSET;
		add(coordinator, EffectKind::ShotBurst, *position, std::nullopt, localPlayerId);
		return true;
	}
	return false;
}

/// Advances effect lifetime and follows living targets without touching network records.
void EffectSystem::update(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network, float deltaTime)
{
	std::vector<engine::core::Entity> expired;
	for (engine::core::Entity entity : entities) {
		auto& effect = coordinator.getComponent<Effect>(entity);
		effect.remaining -= std::max(0.0F, deltaTime);
		if (effect.remaining <= 0.0F) {
			expired.push_back(entity);
			continue;
		}
		if (effect.followPlayer) {
			if (auto position = network.playerPosition(coordinator, *effect.followPlayer)) {
				position->x += PLAYER_WEAPON_OFFSET;
				coordinator.getComponent<engine::core::Position>(entity) = *position;
			}
		} else if (effect.kind == EffectKind::HitSpark && effect.targetId) {
			if (const auto target = network.find(*effect.targetId)) {
				coordinator.getComponent<engine::core::Position>(entity) = coordinator.getComponent<engine::core::Position>(*target);
			}
		}
	}
	for (engine::core::Entity entity : expired) {
		coordinator.destroyEntity(entity);
	}
}

}
