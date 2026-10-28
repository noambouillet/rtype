/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client network system
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "client/network/GameClient.hpp"
#include "client/view/ViewEvent.hpp"
#include "engine/core/ecs/Components.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"
#include "engine/core/systems/InterpolationClock.hpp"

namespace rtype::client {

class ClientNetworkSystem : public engine::core::System {
public:
	static constexpr std::size_t MAX_PENDING_EVENTS = 256;
	static constexpr double MAX_EVENT_LATENESS = 0.25;

	ClientNetworkSystem();
	static engine::core::Signature signature(engine::core::Coordinator& coordinator);
	std::vector<ViewEvent> update(float deltaTime, engine::core::Coordinator& coordinator, network::GameClient& client, std::uint8_t buttons);
	void applySnapshot(engine::core::Coordinator& coordinator, const network::WorldSnapshot& snapshot);
	[[nodiscard]] double renderTime() const;
	[[nodiscard]] std::optional<engine::core::Entity> find(std::uint32_t serverId) const;
	[[nodiscard]] std::optional<engine::core::Position> playerPosition(engine::core::Coordinator& coordinator, std::uint8_t playerId) const;
	[[nodiscard]] std::uint8_t aliveMask() const;
	[[nodiscard]] std::uint8_t connectedMask() const;

private:
	static engine::core::Entity createVisualEntity(engine::core::Coordinator& coordinator, const network::WorldEntity& serverEntity, double time);
	static void updateVisualEntity(engine::core::Coordinator& coordinator, engine::core::Entity visualEntity, const network::WorldEntity& serverEntity, double time);
	static void reveal(engine::core::Coordinator& coordinator, engine::core::Entity visualEntity);
	void revealArrived(engine::core::Coordinator& coordinator);
	void removeDeparted(engine::core::Coordinator& coordinator);
	void queueEvent(const network::WorldEventNotice& notice);
	std::vector<ViewEvent> releaseEvents();

	engine::core::InterpolationClock _clock;
	std::unordered_map<std::uint32_t, engine::core::Entity> _entitiesByServerId;
	std::unordered_map<std::uint32_t, double> _pendingReveals;
	std::unordered_map<std::uint32_t, double> _pendingRemovals;
	std::vector<network::WorldEventNotice> _pendingEvents;
	std::optional<std::uint64_t> _lastAppliedTick;
	std::uint8_t _aliveMask;
	std::uint8_t _connectedMask;
};

}
