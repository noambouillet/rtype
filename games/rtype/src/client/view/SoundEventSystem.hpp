/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client sound cue system
*/

#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "client/view/ClientNetworkSystem.hpp"
#include "client/view/ViewEvent.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"

namespace rtype::client {

class SoundEventSystem : public engine::core::System {
public:
	static constexpr std::string_view MUSIC = "music.ogg";
	static constexpr float MUSIC_VOLUME = 20.0F;

	SoundEventSystem();
	static engine::core::Signature signature(engine::core::Coordinator& coordinator);
	void consume(engine::core::Coordinator& coordinator, const ClientNetworkSystem& network,
		const std::vector<ViewEvent>& events, std::optional<std::uint8_t> localPlayerId);
	static void localFire(engine::core::Coordinator& coordinator);

private:
	static constexpr std::string_view PLAYER_SHOT_SOUND = "ShootPlayer.ogg";
	static constexpr std::string_view ENEMY_SHOT_SOUND = "Shootenemy.ogg";
	static constexpr std::string_view ENEMY_DEATH_SOUND = "EnemyDeath.ogg";
	static constexpr std::string_view PLAYER_DEATH_SOUND = "Ohnan.ogg";
	static constexpr std::string_view PLAYER_JOIN_SOUND = "PlayerJoin.ogg";
	static constexpr std::string_view PLAYER_LEAVE_SOUND = "Tuvasou.ogg";
	static constexpr float PLAYER_SHOT_VOLUME = 100.0F;
	static constexpr float ENEMY_SHOT_VOLUME = 70.0F;
	static constexpr float ENEMY_DEATH_VOLUME = 80.0F;
	static constexpr float PLAYER_DEATH_VOLUME = 150.0F;
	static constexpr float CONNECTION_VOLUME = 150.0F;

	static void emit(engine::core::Coordinator& coordinator, std::string_view name, float volume);
	static void handle(engine::core::Coordinator& coordinator, const SpawnEvent& event, std::optional<std::uint8_t> localPlayerId);
	static void handle(engine::core::Coordinator& coordinator, const DestroyEvent& event);
	static void handle(engine::core::Coordinator& coordinator, const DeathEvent& event);
	void updateConnections(engine::core::Coordinator& coordinator, std::uint8_t connected);

	std::uint8_t _connectedMask;
};

}
