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

#include "client/network/GameClient.hpp"
#include "client/view/ViewEvent.hpp"
#include "engine/assets/AssetManager.hpp"
#include "engine/core/ecs/Components.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"

namespace rtype::client {

class ClientNetworkSystem : public engine::core::System {
public:
	ClientNetworkSystem();
	void setAssetManager(const engine::assets::AssetManager& assets);
	static engine::core::Signature signature(engine::core::Coordinator& coordinator);
	std::vector<ViewEvent> update(engine::core::Coordinator& coordinator, network::GameClient& client, std::uint8_t buttons);
	void applySnapshot(engine::core::Coordinator& coordinator, const network::WorldSnapshot& snapshot);
	[[nodiscard]] std::optional<engine::core::Entity> find(std::uint32_t serverId) const;
	[[nodiscard]] std::optional<engine::core::Position> playerPosition(engine::core::Coordinator& coordinator, std::uint8_t playerId) const;
	[[nodiscard]] std::uint8_t aliveMask() const;
	[[nodiscard]] std::uint8_t connectedMask() const;

private:
	engine::core::Entity createVisualEntity(engine::core::Coordinator& coordinator, const network::WorldEntity& serverEntity);
	void updateVisualEntity(engine::core::Coordinator& coordinator, engine::core::Entity visualEntity, const network::WorldEntity& serverEntity);

	const engine::assets::AssetManager* _assets = nullptr;
	std::unordered_map<std::uint32_t, engine::core::Entity> _entitiesByServerId;
	std::optional<std::uint64_t> _lastAppliedTick;
	std::uint8_t _aliveMask;
	std::uint8_t _connectedMask;
};

}
