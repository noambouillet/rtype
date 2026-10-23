/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Server game world: owns the ECS and runs one gameplay tick
*/

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "Coordinator.hpp"
#include "server/ecs/Systems/ReplicationSystem.hpp"
#include "server/game/GameConfig.hpp"
#include "server/game/WorldEvent.hpp"
#include "server/game/WorldEventCollector.hpp"

namespace rtype {

class GameWorld {
public:
	static constexpr std::size_t MAX_PLAYERS = config::MAX_PLAYERS;

	struct Input {
		std::array<std::uint8_t, MAX_PLAYERS> buttons;
		std::uint8_t connectedMask;
	};

	using EntityState = ecs::ReplicatedState;

	GameWorld();
	std::vector<WorldEvent> update(const Input& input, float deltaTime);
	[[nodiscard]] std::uint8_t aliveMask() const;
	[[nodiscard]] std::vector<EntityState> replicatedEntities();

private:
	void registerComponents();
	void registerSystems();
	void syncPlayerEntities(const Input& input, WorldEventCollector& events);
	void runSystems(const Input& input, float deltaTime, WorldEventCollector& events);

	ecs::Coordinator _coordinator;
	std::array<std::optional<ecs::Entity>, MAX_PLAYERS> _playerEntities;
};

}
