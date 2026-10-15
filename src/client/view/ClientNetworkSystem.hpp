/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client network system
*/

#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "Components.hpp"
#include "Coordinator.hpp"
#include "System.hpp"
#include "client/network/GameClient.hpp"
#include "client/view/ViewEvent.hpp"

namespace rtype::client {

class ClientNetworkSystem : public ecs::System {
public:
	ClientNetworkSystem();
	ecs::Signature signature(ecs::Coordinator& coordinator);
	std::vector<ViewEvent> update(ecs::Coordinator& coordinator, network::GameClient& client, std::uint8_t buttons);
	void applySnapshot(ecs::Coordinator& coordinator, const network::WorldSnapshot& snapshot);
	[[nodiscard]] std::optional<ecs::Entity> find(std::uint32_t serverId) const;
	[[nodiscard]] std::optional<ecs::Position> playerPosition(ecs::Coordinator& coordinator, std::uint8_t playerId) const;
	[[nodiscard]] std::uint8_t aliveMask() const;
	[[nodiscard]] std::uint8_t connectedMask() const;

private:
	ecs::Entity createVisualEntity(ecs::Coordinator& coordinator, const network::WorldEntity& serverEntity);
	void updateVisualEntity(ecs::Coordinator& coordinator, ecs::Entity visualEntity, const network::WorldEntity& serverEntity);

	std::unordered_map<std::uint32_t, ecs::Entity> _entitiesByServerId;
	std::optional<std::uint64_t> _lastAppliedTick;
	std::uint8_t _aliveMask;
	std::uint8_t _connectedMask;
};

}
