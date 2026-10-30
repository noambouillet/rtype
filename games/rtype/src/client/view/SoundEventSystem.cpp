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
#include "engine/audio/Mixer.hpp"
#include "engine/core/ecs/Components.hpp"
#include "game/GameTypes.hpp"

namespace rtype::client {

SoundEventSystem::SoundEventSystem() : _assets(std::nullopt), _connectedMask(0), _listenerX(PAN_RANGE)
{
}

void SoundEventSystem::setAssetManager(const engine::assets::AssetManager& assets)
{
	_assets = std::cref(assets);
}

engine::core::Signature SoundEventSystem::signature(engine::core::Coordinator& coordinator)
{
	engine::core::Signature signature;
	signature.set(coordinator.getComponentType<engine::audio::Sound>());
	return signature;
}

std::string_view SoundEventSystem::resolveSound(std::string_view key) const
{
	if (_assets && _assets->get().hasSound(std::string(key))) {
		return _assets->get().getSoundPath(std::string(key));
	}
	return {};
}

/// Returns the pan of a sound at x, heard from the local ship.
float SoundEventSystem::panAt(float x) const
{
	return engine::audio::Mixer::stereoPan(x, _listenerX, PAN_RANGE) * PAN_WIDTH;
}

/// Moves the listener to the local ship. It stays at its last place while the ship is dead.
void SoundEventSystem::updateListener(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network, std::optional<std::uint8_t> localPlayerId)
{
	if (!localPlayerId || (network.aliveMask() & (1 << *localPlayerId)) == 0) {
		return;
	}
	if (const auto position = network.playerPosition(coordinator, *localPlayerId)) {
		_listenerX = position->x;
	}
}

/// Queues a one-shot sound entity for the audio system to play this frame.
void SoundEventSystem::emit(engine::core::Coordinator& coordinator, std::string_view name, float volume, engine::audio::Bus bus, float pan)
{
	if (name.empty()) {
		return;
	}
	const engine::core::Entity entity = coordinator.createEntity();
	coordinator.addComponent(entity, engine::audio::Sound{.name = std::string(name), .volume = volume, .bus = bus, .pan = pan});
}

void SoundEventSystem::handle(engine::core::Coordinator& coordinator, const SpawnEvent& event, std::optional<std::uint8_t> localPlayerId)
{
	if (event.kind == game::EntityType::EnemyProjectile) {
		emit(coordinator, resolveSound(ENEMY_SHOT_SOUND), ENEMY_SHOT_VOLUME, engine::audio::Bus::Sfx, panAt(event.x));
	} else if (event.kind == game::EntityType::PlayerProjectile && (!localPlayerId || event.owner != *localPlayerId)) {
		emit(coordinator, resolveSound(PLAYER_SHOT_SOUND), PLAYER_SHOT_VOLUME, engine::audio::Bus::Sfx, panAt(event.x));
	}
}

void SoundEventSystem::handle(engine::core::Coordinator& coordinator, const DestroyEvent& event)
{
	if (event.kind == game::EntityType::Enemy && destroyedOnscreen(event)) {
		emit(coordinator, resolveSound(ENEMY_DEATH_SOUND), ENEMY_DEATH_VOLUME, engine::audio::Bus::Sfx, panAt(event.x));
	}
}

void SoundEventSystem::handle(engine::core::Coordinator& coordinator, const DeathEvent& event)
{
	emit(coordinator, resolveSound(PLAYER_DEATH_SOUND), PLAYER_DEATH_VOLUME, engine::audio::Bus::Sfx, panAt(event.x));
}

/// Plays one sound per player gained or lost by the connected players mask.
void SoundEventSystem::updateConnections(engine::core::Coordinator& coordinator, std::uint8_t connected)
{
	const int joined = std::popcount(static_cast<std::uint8_t>(connected & ~_connectedMask));
	const int left = std::popcount(static_cast<std::uint8_t>(_connectedMask & ~connected));
	for (int player = 0; player < joined; player++) {
		emit(coordinator, resolveSound(PLAYER_JOIN_SOUND), CONNECTION_VOLUME, engine::audio::Bus::Ui, 0.0F);
	}
	for (int player = 0; player < left; player++) {
		emit(coordinator, resolveSound(PLAYER_LEAVE_SOUND), CONNECTION_VOLUME, engine::audio::Bus::Ui, 0.0F);
	}
	_connectedMask = connected;
}

/// Converts game notices and roster changes into sound entities.
void SoundEventSystem::consume(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network,
	const std::vector<ViewEvent>& events, std::optional<std::uint8_t> localPlayerId)
{
	updateListener(coordinator, network, localPlayerId);
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
	emit(coordinator, resolveSound(PLAYER_SHOT_SOUND), PLAYER_SHOT_VOLUME, engine::audio::Bus::Sfx, 0.0F);
}

}
