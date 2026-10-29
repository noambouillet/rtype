/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client sound cue system
*/

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "client/view/ClientNetworkSystem.hpp"
#include "client/view/ViewEvent.hpp"
#include "engine/assets/AssetManager.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"

namespace rtype::client {

class SoundEventSystem : public engine::core::System {
public:
	static constexpr std::string_view MUSIC = "music";
	static constexpr float MUSIC_VOLUME = 20.0F;

	SoundEventSystem();
	void setAssetManager(const engine::assets::AssetManager& assets);
	static engine::core::Signature signature(engine::core::Coordinator& coordinator);
	void consume(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network,
		const std::vector<ViewEvent>& events, std::optional<std::uint8_t> localPlayerId);
	void localFire(engine::core::Coordinator& coordinator);

private:
	static constexpr std::string_view PLAYER_SHOT_SOUND = "ShootPlayer";
	static constexpr std::string_view ENEMY_SHOT_SOUND = "Shootenemy";
	static constexpr std::string_view ENEMY_DEATH_SOUND = "EnemyDeath";
	static constexpr std::string_view PLAYER_DEATH_SOUND = "Ohnan";
	static constexpr std::string_view PLAYER_JOIN_SOUND = "PlayerJoin";
	static constexpr std::string_view PLAYER_LEAVE_SOUND = "Tuvasou";
	static constexpr float PLAYER_SHOT_VOLUME = 100.0F;
	static constexpr float ENEMY_SHOT_VOLUME = 70.0F;
	static constexpr float ENEMY_DEATH_VOLUME = 80.0F;
	static constexpr float PLAYER_DEATH_VOLUME = 150.0F;
	static constexpr float CONNECTION_VOLUME = 150.0F;

	[[nodiscard]] std::string resolveSound(std::string_view key) const;
	void emit(engine::core::Coordinator& coordinator, std::string_view name, float volume);
	void handle(engine::core::Coordinator& coordinator, const SpawnEvent& event, std::optional<std::uint8_t> localPlayerId);
	void handle(engine::core::Coordinator& coordinator, const DestroyEvent& event);
	void handle(engine::core::Coordinator& coordinator, const DeathEvent& event);
	void updateConnections(engine::core::Coordinator& coordinator, std::uint8_t connected);

	const engine::assets::AssetManager* _assets = nullptr;
	std::uint8_t _connectedMask;
};

}
