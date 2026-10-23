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

#include "client/view/ClientComponents.hpp"
#include "client/view/ClientNetworkSystem.hpp"
#include "client/view/LocalFirePredictor.hpp"
#include "client/view/ViewEvent.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"

namespace rtype::client {

class EffectSystem : public engine::core::System {
public:
	EffectSystem();
	engine::core::Signature signature(engine::core::Coordinator& coordinator);
	void consume(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network,
		const std::vector<ViewEvent>& events, std::optional<std::uint8_t> localPlayerId);
	bool predictLocalFire(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network,
		std::uint8_t buttons, std::optional<std::uint8_t> localPlayerId);
	void update(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network, float deltaTime);

private:
	static constexpr std::size_t MAX_EFFECTS = 128;
	static constexpr float PLAYER_WEAPON_OFFSET = 32.0F;

	void add(engine::core::Coordinator& coordinator, EffectKind kind, engine::core::Position position,
		std::optional<std::uint32_t> targetId = std::nullopt,
		std::optional<std::uint8_t> followPlayer = std::nullopt);
	void removeHitSpark(engine::core::Coordinator& coordinator, std::uint32_t targetId);
	void handle(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network,
		const SpawnEvent& event, std::optional<std::uint8_t> localPlayerId);
	void handle(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network, const DestroyEvent& event);
	void handle(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network, const DamageEvent& event);
	void handle(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network, const DeathEvent& event);

	LocalFirePredictor _firePredictor;
};

}
