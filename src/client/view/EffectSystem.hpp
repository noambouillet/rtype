/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client effect system
*/

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "Coordinator.hpp"
#include "System.hpp"
#include "client/view/ClientComponents.hpp"
#include "client/view/ClientNetworkSystem.hpp"
#include "client/view/LocalFirePredictor.hpp"
#include "client/view/ViewEvent.hpp"

namespace rtype::client {

class EffectSystem : public ecs::System {
public:
	EffectSystem();
	ecs::Signature signature(ecs::Coordinator& coordinator);
	void consume(ecs::Coordinator& coordinator, const ClientNetworkSystem& network,
		const std::vector<ViewEvent>& events, std::optional<std::uint8_t> localPlayerId);
	bool predictLocalFire(ecs::Coordinator& coordinator, const ClientNetworkSystem& network,
		std::uint8_t buttons, std::optional<std::uint8_t> localPlayerId);
	void update(ecs::Coordinator& coordinator, const ClientNetworkSystem& network, float deltaTime);

private:
	static constexpr std::size_t MAX_EFFECTS = 128;
	static constexpr float PLAYER_WEAPON_OFFSET = 32.0F;

	void add(ecs::Coordinator& coordinator, EffectKind kind, ecs::Position position,
		std::optional<std::uint32_t> targetId = std::nullopt,
		std::optional<std::uint8_t> followPlayer = std::nullopt);
	void removeHitSpark(ecs::Coordinator& coordinator, std::uint32_t targetId);
	void handle(ecs::Coordinator& coordinator, const ClientNetworkSystem& network,
		const SpawnEvent& event, std::optional<std::uint8_t> localPlayerId);
	void handle(ecs::Coordinator& coordinator, const ClientNetworkSystem& network, const DestroyEvent& event);
	void handle(ecs::Coordinator& coordinator, const ClientNetworkSystem& network, const DamageEvent& event);
	void handle(ecs::Coordinator& coordinator, const ClientNetworkSystem& network, const DeathEvent& event);

	LocalFirePredictor _firePredictor;
};

}
