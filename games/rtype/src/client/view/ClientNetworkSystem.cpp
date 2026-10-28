/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client network system
*/

#include "ClientNetworkSystem.hpp"

#include <cstddef>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "client/GameConfig.hpp"
#include "client/view/ClientComponents.hpp"
#include "client/view/SpriteCatalog.hpp"
#include "engine/core/ecs/Components.hpp"
#include "engine/core/systems/InterpolationSystem.hpp"
#include "engine/render/Components.hpp"
#include "utils/network/Protocol.hpp"

namespace rtype::client {

ClientNetworkSystem::ClientNetworkSystem()
	: _clock(client_config::INTERPOLATION_DELAY), _entitiesByServerId{}, _pendingReveals{}, _pendingRemovals{}, _pendingEvents{}, _lastAppliedTick{}, _aliveMask(0), _connectedMask(0)
{
}

engine::core::Signature ClientNetworkSystem::signature(engine::core::Coordinator& coordinator)
{
	engine::core::Signature required;
	required.set(coordinator.getComponentType<NetworkIdentity>());
	return required;
}

/// Converts a server tick into a time stamp, in seconds, on the server's timeline.
static double tickTime(std::uint64_t tick)
{
	return static_cast<double>(tick) * network::TICK_SECONDS;
}

/// Converts a validated protocol notice into a presentation event.
static ViewEvent toViewEvent(const network::WorldEventNotice& notice)
{
	if (const auto* spawn = std::get_if<network::EntitySpawnEvent>(&notice.data)) {
		const network::WorldEntity& entity = spawn->entity;
		return SpawnEvent{.id = entity.entityId, .kind = entity.entityType, .owner = entity.ownerPlayerId, .x = entity.x, .y = entity.y};
	}
	if (const auto* destroy = std::get_if<network::EntityDestroyEvent>(&notice.data)) {
		return DestroyEvent{.id = destroy->entityId, .kind = destroy->kind, .x = destroy->x, .y = destroy->y};
	}
	if (const auto* damage = std::get_if<network::EntityDamageEvent>(&notice.data)) {
		return DamageEvent{damage->entityId};
	}
	const auto& death = std::get<network::PlayerDeathEvent>(notice.data);
	return DeathEvent{.id = death.entityId, .x = death.x, .y = death.y};
}

/// Polls the session, records a new authoritative snapshot, and returns the notices due this frame.
/// The view plays the server timeline INTERPOLATION_DELAY seconds late, so positions can be
/// blended between two snapshots. Spawns, removals, and notices wait for that same moment so
/// they stay in step with the positions on screen.
std::vector<ViewEvent> ClientNetworkSystem::update(float deltaTime, engine::core::Coordinator& coordinator, network::GameClient& client, std::uint8_t buttons)
{
	client.tick(buttons);
	_clock.advance(deltaTime);
	if (const auto& snapshot = client.worldSnapshot()) {
		applySnapshot(coordinator, *snapshot);
	}
	for (const network::WorldEventNotice& notice : client.takeEvents()) {
		queueEvent(notice);
	}
	revealArrived(coordinator);
	removeDeparted(coordinator);
	return releaseEvents();
}

/// Records the positions of a newer snapshot and schedules the entities it adds or drops.
void ClientNetworkSystem::applySnapshot(engine::core::Coordinator& coordinator, const network::WorldSnapshot& snapshot)
{
	if (_lastAppliedTick && snapshot.tick <= *_lastAppliedTick) {
		return;
	}
	const double time = tickTime(snapshot.tick);
	_clock.observe(time);
	std::unordered_set<std::uint32_t> seenServerIds;
	seenServerIds.reserve(snapshot.entities.size());
	for (const network::WorldEntity& serverEntity : snapshot.entities) {
		if (!seenServerIds.insert(serverEntity.entityId).second) {
			continue;
		}
		const auto entityEntry = _entitiesByServerId.find(serverEntity.entityId);
		if (entityEntry == _entitiesByServerId.end()) {
			_entitiesByServerId.emplace(serverEntity.entityId, createVisualEntity(coordinator, serverEntity, time));
			_pendingReveals.emplace(serverEntity.entityId, time);
		} else {
			updateVisualEntity(coordinator, entityEntry->second, serverEntity, time);
			_pendingRemovals.erase(serverEntity.entityId);
		}
	}
	for (const auto& [serverId, entity] : _entitiesByServerId) {
		if (!seenServerIds.contains(serverId)) {
			_pendingRemovals.try_emplace(serverId, time);
		}
	}
	_aliveMask = snapshot.aliveMask;
	_connectedMask = snapshot.connectedMask;
	_lastAppliedTick = snapshot.tick;
}

/// Returns the server time, in seconds, that the view shows this frame.
double ClientNetworkSystem::renderTime() const
{
	return _clock.renderTime();
}

/// Creates a hidden presentation entity for an entity first seen in a snapshot.
/// It stays invisible until the view reaches "time", so it does not show up before its neighbours.
engine::core::Entity ClientNetworkSystem::createVisualEntity(engine::core::Coordinator& coordinator, const network::WorldEntity& serverEntity, double time)
{
	const engine::core::Entity visualEntity = coordinator.createEntity();
	coordinator.addComponent(visualEntity, engine::core::Position{.x = serverEntity.x, .y = serverEntity.y});
	coordinator.addComponent(visualEntity, engine::core::Interpolated{.samples = {{.time = time, .x = serverEntity.x, .y = serverEntity.y}}});
	const NetworkIdentity identity{.serverId = serverEntity.entityId, .kind = serverEntity.entityType, .owner = serverEntity.ownerPlayerId, .variant = serverEntity.spriteId};
	coordinator.addComponent(visualEntity, identity);
	return visualEntity;
}

/// Records the authoritative state of an existing presentation entity at "time".
void ClientNetworkSystem::updateVisualEntity(engine::core::Coordinator& coordinator, engine::core::Entity visualEntity, const network::WorldEntity& serverEntity, double time)
{
	engine::core::InterpolationSystem::addSample(coordinator.getComponent<engine::core::Interpolated>(visualEntity), {.time = time, .x = serverEntity.x, .y = serverEntity.y});
	auto& identity = coordinator.getComponent<NetworkIdentity>(visualEntity);
	const bool appearanceChanged = identity.kind != serverEntity.entityType || identity.variant != serverEntity.spriteId;
	identity = {.serverId = serverEntity.entityId, .kind = serverEntity.entityType, .owner = serverEntity.ownerPlayerId, .variant = serverEntity.spriteId};
	if (appearanceChanged && coordinator.hasComponent<engine::render::Sprite>(visualEntity)) {
		coordinator.getComponent<engine::render::Sprite>(visualEntity) = spriteFor(serverEntity.entityType, serverEntity.spriteId);
		const auto animation = animationFor(serverEntity.entityType);
		if (animation && !coordinator.hasComponent<engine::render::Animation>(visualEntity)) {
			coordinator.addComponent(visualEntity, *animation);
		} else if (!animation && coordinator.hasComponent<engine::render::Animation>(visualEntity)) {
			coordinator.removeComponent<engine::render::Animation>(visualEntity);
		}
	}
}

/// Gives a hidden presentation entity the sprite matching its current identity.
void ClientNetworkSystem::reveal(engine::core::Coordinator& coordinator, engine::core::Entity visualEntity)
{
	const NetworkIdentity& identity = coordinator.getComponent<NetworkIdentity>(visualEntity);
	coordinator.addComponent(visualEntity, spriteFor(identity.kind, identity.variant));
	if (auto animation = animationFor(identity.kind)) {
		coordinator.addComponent(visualEntity, std::move(*animation));
	}
}

/// Shows the entities whose first snapshot the view has reached.
void ClientNetworkSystem::revealArrived(engine::core::Coordinator& coordinator)
{
	const double now = _clock.renderTime();
	for (auto entry = _pendingReveals.begin(); entry != _pendingReveals.end();) {
		const auto visual = _entitiesByServerId.find(entry->first);
		if (visual == _entitiesByServerId.end()) {
			entry = _pendingReveals.erase(entry);
		} else if (entry->second <= now) {
			reveal(coordinator, visual->second);
			entry = _pendingReveals.erase(entry);
		} else {
			entry++;
		}
	}
}

/// Destroys the entities whose disappearance the view has reached.
void ClientNetworkSystem::removeDeparted(engine::core::Coordinator& coordinator)
{
	const double now = _clock.renderTime();
	for (auto entry = _pendingRemovals.begin(); entry != _pendingRemovals.end();) {
		if (entry->second > now) {
			entry++;
			continue;
		}
		if (const auto visual = _entitiesByServerId.find(entry->first); visual != _entitiesByServerId.end()) {
			coordinator.destroyEntity(visual->second);
			_entitiesByServerId.erase(visual);
		}
		_pendingReveals.erase(entry->first);
		entry = _pendingRemovals.erase(entry);
	}
}

/// Holds a notice until the view reaches its tick.
///
/// A notice stamped further ahead of the estimated server time than the clock's jump
/// threshold cannot come from a sane server, so it is dropped instead of waiting forever.
void ClientNetworkSystem::queueEvent(const network::WorldEventNotice& notice)
{
	const double time = tickTime(notice.tick);
	if (_clock.isStarted() && (time > _clock.sourceTime() + engine::core::InterpolationClock::SNAP_THRESHOLD || time < _clock.renderTime() - MAX_EVENT_LATENESS)) {
		return;
	}
	_pendingEvents.push_back(notice);
}

/// Returns, in arrival order, the notices whose tick the view has reached.
std::vector<ViewEvent> ClientNetworkSystem::releaseEvents()
{
	const double now = _clock.renderTime();
	const std::size_t overflow = _pendingEvents.size() > MAX_PENDING_EVENTS ? _pendingEvents.size() - MAX_PENDING_EVENTS : 0;
	std::vector<ViewEvent> released;
	std::vector<network::WorldEventNotice> waiting;
	for (std::size_t index = 0; index < _pendingEvents.size(); index++) {
		const network::WorldEventNotice& notice = _pendingEvents[index];
		if (!_clock.isStarted() || index < overflow || tickTime(notice.tick) <= now) {
			released.push_back(toViewEvent(notice));
		} else {
			waiting.push_back(notice);
		}
	}
	_pendingEvents.swap(waiting);
	return released;
}

std::optional<engine::core::Entity> ClientNetworkSystem::find(std::uint32_t serverId) const
{
	const auto entry = _entitiesByServerId.find(serverId);
	if (entry == _entitiesByServerId.end()) {
		return std::nullopt;
	}
	return entry->second;
}

/// Locates an authoritative player's visible ship without exposing protocol records.
std::optional<engine::core::Position> ClientNetworkSystem::playerPosition(engine::core::Coordinator& coordinator, std::uint8_t playerId) const
{
	for (const engine::core::Entity entity : entities) {
		const NetworkIdentity& identity = coordinator.getComponent<NetworkIdentity>(entity);
		if (identity.kind == game::EntityType::Player && identity.owner == playerId) {
			return coordinator.getComponent<engine::core::Position>(entity);
		}
	}
	return std::nullopt;
}

std::uint8_t ClientNetworkSystem::aliveMask() const
{
	return _aliveMask;
}

std::uint8_t ClientNetworkSystem::connectedMask() const
{
	return _connectedMask;
}

}
