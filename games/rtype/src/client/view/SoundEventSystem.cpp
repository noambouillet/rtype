/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client sound cue system
*/

#include "SoundEventSystem.hpp"

#include <bit>
#include <string>
#include <variant>

#include "engine/audio/Components.hpp"
#include "engine/core/ecs/Components.hpp"
#include "game/GameTypes.hpp"

namespace rtype::client {

SoundEventSystem::SoundEventSystem() : _connectedMask(0)
{
}

engine::core::Signature SoundEventSystem::signature(engine::core::Coordinator& coordinator)
{
	engine::core::Signature signature;
	signature.set(coordinator.getComponentType<engine::audio::Sound>());
	return signature;
}

/// Queues a one-shot sound entity for the audio system to play this frame.
void SoundEventSystem::emit(engine::core::Coordinator& coordinator, std::string_view name, float volume)
{
	const engine::core::Entity entity = coordinator.createEntity();
	coordinator.addComponent(entity, engine::audio::Sound{.name = std::string(name), .volume = volume});
}

void SoundEventSystem::handle(engine::core::Coordinator& coordinator, const SpawnEvent& event, std::optional<std::uint8_t> localPlayerId)
{
	if (event.kind == game::EntityType::EnemyProjectile) {
		emit(coordinator, ENEMY_SHOT_SOUND, ENEMY_SHOT_VOLUME);
	} else if (event.kind == game::EntityType::PlayerProjectile && (!localPlayerId || event.owner != *localPlayerId)) {
		emit(coordinator, PLAYER_SHOT_SOUND, PLAYER_SHOT_VOLUME);
	}
}

void SoundEventSystem::handle(engine::core::Coordinator& coordinator, const DestroyEvent& event)
{
	if (event.kind == game::EntityType::Enemy && destroyedOnscreen(event)) {
		emit(coordinator, ENEMY_DEATH_SOUND, ENEMY_DEATH_VOLUME);
	}
}

void SoundEventSystem::handle(engine::core::Coordinator& coordinator, [[maybe_unused]] const DeathEvent& event)
{
	emit(coordinator, PLAYER_DEATH_SOUND, PLAYER_DEATH_VOLUME);
}

/// Plays one sound per player gained or lost by the connected players mask.
void SoundEventSystem::updateConnections(engine::core::Coordinator& coordinator, std::uint8_t connected)
{
	const int joined = std::popcount(static_cast<std::uint8_t>(connected & ~_connectedMask));
	const int left = std::popcount(static_cast<std::uint8_t>(_connectedMask & ~connected));
	for (int player = 0; player < joined; player++) {
		emit(coordinator, PLAYER_JOIN_SOUND, CONNECTION_VOLUME);
	}
	for (int player = 0; player < left; player++) {
		emit(coordinator, PLAYER_LEAVE_SOUND, CONNECTION_VOLUME);
	}
	_connectedMask = connected;
}

/// Converts game notices and roster changes into sound entities.
void SoundEventSystem::consume(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network,
	const std::vector<ViewEvent>& events, std::optional<std::uint8_t> localPlayerId)
{
	updateConnections(coordinator, network.connectedMask());
	for (const ViewEvent& event : events) {
		if (const auto* spawn = std::get_if<SpawnEvent>(&event)) {
			handle(coordinator, *spawn, localPlayerId);
		} else if (const auto* destroy = std::get_if<DestroyEvent>(&event)) {
			handle(coordinator, *destroy);
		} else if (const auto* death = std::get_if<DeathEvent>(&event)) {
			handle(coordinator, *death);
		}
	}
}

/// Plays the local shot sound as soon as the predicted burst starts.
void SoundEventSystem::localFire(engine::core::Coordinator& coordinator)
{
	emit(coordinator, PLAYER_SHOT_SOUND, PLAYER_SHOT_VOLUME);
}

}
