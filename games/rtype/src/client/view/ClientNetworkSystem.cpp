/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client network system
*/

#include "ClientNetworkSystem.hpp"

#include <unordered_set>
#include <utility>
#include <variant>

#include "client/view/ClientComponents.hpp"
#include "client/view/SpriteCatalog.hpp"
#include "engine/core/ecs/Components.hpp"

namespace rtype::client {

ClientNetworkSystem::ClientNetworkSystem() : _entitiesByServerId{}, _lastAppliedTick{}, _aliveMask(0), _connectedMask(0)
{
}

engine::core::Signature ClientNetworkSystem::signature(engine::core::Coordinator& coordinator)
{
	engine::core::Signature required;
	required.set(coordinator.getComponentType<NetworkIdentity>());
	return required;
}

/// Converts a validated protocol notice into a presentation event.
static ViewEvent toViewEvent(const network::WorldEventNotice& notice)
{
	if (const auto* spawn = std::get_if<network::EntitySpawnEvent>(&notice.data)) {
		const network::WorldEntity& entity = spawn->entity;
		return SpawnEvent{entity.entityId, entity.entityType, entity.ownerPlayerId, entity.x, entity.y};
	}
	if (const auto* destroy = std::get_if<network::EntityDestroyEvent>(&notice.data)) {
		return DestroyEvent{destroy->entityId, destroy->kind, destroy->x, destroy->y};
	}
	if (const auto* damage = std::get_if<network::EntityDamageEvent>(&notice.data)) {
		return DamageEvent{damage->entityId};
	}
	const auto& death = std::get<network::PlayerDeathEvent>(notice.data);
	return DeathEvent{death.entityId, death.x, death.y};
}

/// Polls the session, reconciles a new authoritative snapshot, and transfers notices.
std::vector<ViewEvent> ClientNetworkSystem::update(engine::core::Coordinator& coordinator, network::GameClient& client, std::uint8_t buttons)
{
	client.tick(buttons);
	if (const auto& snapshot = client.worldSnapshot()) {
		applySnapshot(coordinator, *snapshot);
	}
	std::vector<ViewEvent> viewEvents;
	for (const network::WorldEventNotice& notice : client.takeEvents()) {
		viewEvents.push_back(toViewEvent(notice));
	}
	return viewEvents;
}

/// Mirrors only entity appearance and position from a newer full snapshot.
void ClientNetworkSystem::applySnapshot(engine::core::Coordinator& coordinator, const network::WorldSnapshot& snapshot)
{
	if (_lastAppliedTick && snapshot.tick <= *_lastAppliedTick) {
		return;
	}
	std::unordered_set<std::uint32_t> seenServerIds;
	seenServerIds.reserve(snapshot.entities.size());
	for (const network::WorldEntity& serverEntity : snapshot.entities) {
		if (!seenServerIds.insert(serverEntity.entityId).second) {
			continue;
		}
		const auto entityEntry = _entitiesByServerId.find(serverEntity.entityId);
		if (entityEntry == _entitiesByServerId.end()) {
			_entitiesByServerId.emplace(serverEntity.entityId, createVisualEntity(coordinator, serverEntity));
		} else {
			updateVisualEntity(coordinator, entityEntry->second, serverEntity);
		}
	}
	for (auto entry = _entitiesByServerId.begin(); entry != _entitiesByServerId.end();) {
		if (!seenServerIds.contains(entry->first)) {
			coordinator.destroyEntity(entry->second);
			entry = _entitiesByServerId.erase(entry);
		} else {
			entry++;
		}
	}
	_aliveMask = snapshot.aliveMask;
	_connectedMask = snapshot.connectedMask;
	_lastAppliedTick = snapshot.tick;
}

/// Creates the client presentation components for an entity first seen in a snapshot.
engine::core::Entity ClientNetworkSystem::createVisualEntity(engine::core::Coordinator& coordinator, const network::WorldEntity& serverEntity)
{
	const engine::core::Entity visualEntity = coordinator.createEntity();
	coordinator.addComponent(visualEntity, engine::core::Position{serverEntity.x, serverEntity.y});
	coordinator.addComponent(visualEntity, spriteFor(serverEntity.entityType, serverEntity.spriteId));
	if (auto animation = animationFor(serverEntity.entityType)) {
		coordinator.addComponent(visualEntity, std::move(*animation));
	}
	const NetworkIdentity identity{serverEntity.entityId, serverEntity.entityType,
		serverEntity.ownerPlayerId, serverEntity.spriteId};
	coordinator.addComponent(visualEntity, identity);
	return visualEntity;
}

/// Refreshes an existing presentation entity from authoritative state.
void ClientNetworkSystem::updateVisualEntity(engine::core::Coordinator& coordinator, engine::core::Entity visualEntity, const network::WorldEntity& serverEntity)
{
	coordinator.getComponent<engine::core::Position>(visualEntity) = {serverEntity.x, serverEntity.y};
	NetworkIdentity& identity = coordinator.getComponent<NetworkIdentity>(visualEntity);
	if (identity.kind != serverEntity.entityType || identity.variant != serverEntity.spriteId) {
		coordinator.getComponent<engine::core::Sprite>(visualEntity) = spriteFor(serverEntity.entityType, serverEntity.spriteId);
		const auto animation = animationFor(serverEntity.entityType);
		if (animation && !coordinator.hasComponent<engine::core::Animation>(visualEntity)) {
			coordinator.addComponent(visualEntity, *animation);
		} else if (!animation && coordinator.hasComponent<engine::core::Animation>(visualEntity)) {
			coordinator.removeComponent<engine::core::Animation>(visualEntity);
		}
	}
	identity = {serverEntity.entityId, serverEntity.entityType, serverEntity.ownerPlayerId, serverEntity.spriteId};
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
