/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client effect system
*/

#include "EffectSystem.hpp"

#include <algorithm>
#include <span>
#include <string>
#include <string_view>
#include <variant>

#include "client/view/SpriteRegions.hpp"
#include "engine/core/ecs/Components.hpp"
#include "engine/render/Components.hpp"
#include "game/GameTypes.hpp"

namespace rtype::client {

struct EffectArt {
	std::string_view texture;
	std::span<const engine::render::Rect> frames;
	float frameSeconds;
};

/// Chooses asset frames for a temporary presentation effect.
static EffectArt artFor(EffectKind kind)
{
	switch (kind) {
		case EffectKind::ShotBurst:
			return {sprites::SHOT_SHEET, sprites::SHOT_BURST_FRAMES, 0.055F};
		case EffectKind::HitSpark:
			return {sprites::SHOT_SHEET, sprites::HIT_SPARK_FRAMES, 0.04F};
		case EffectKind::MobExplosion:
			return {sprites::EXPLOSION_SHEET, sprites::MOB_EXPLOSION_FRAMES, 0.025F};
		case EffectKind::PlayerExplosion:
			return {sprites::EXPLOSION_SHEET, sprites::PLAYER_EXPLOSION_FRAMES, 0.07F};
	}
	return {};
}

EffectSystem::EffectSystem() : _firePredictor{}
{
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
	if (entities.size() >= MAX_EFFECTS) {
		coordinator.destroyEntity(*entities.begin());
	}
	const EffectArt art = artFor(kind);
	if (art.frames.empty()) {
		return;
	}
	const engine::render::Rect first = art.frames.front();
	const engine::core::Entity entity = coordinator.createEntity();
	coordinator.addComponent(entity, position);
	coordinator.addComponent(entity, engine::render::Sprite{std::string(art.texture), first,
										 static_cast<float>(first.size.x) / 2.0F, static_cast<float>(first.size.y) / 2.0F, 2.0F, 0.0F, 10});
	coordinator.addComponent(entity, engine::render::Animation{{art.frames.begin(), art.frames.end()}, art.frameSeconds, 0.0F, false});
	coordinator.addComponent(entity, Effect{kind, art.frameSeconds * static_cast<float>(art.frames.size()), targetId, followPlayer});
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
		add(coordinator, EffectKind::ShotBurst, {event.x, event.y}, event.id, event.owner);
	}
}

void EffectSystem::handle(engine::core::Coordinator& coordinator, [[maybe_unused]] const ClientNetworkSystem& network,
	const DestroyEvent& event)
{
	removeHitSpark(coordinator, event.id);
	if (event.kind == game::EntityType::Enemy && destroyedOnscreen(event)) {
		add(coordinator, EffectKind::MobExplosion, {event.x, event.y}, event.id);
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
	add(coordinator, EffectKind::PlayerExplosion, {event.x, event.y}, event.id);
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
		Effect& effect = coordinator.getComponent<Effect>(entity);
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
