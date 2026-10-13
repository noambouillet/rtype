/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Temporary client-side visual effects
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "AssetManager.hpp"
#include "LocalFirePredictor.hpp"
#include "SoundPlayer.hpp"
#include "client/GameConfig.hpp"

namespace rtype::network {

struct EntitySpawnEvent;
struct EntityDestroyEvent;
struct EntityDamageEvent;
struct PlayerDeathEvent;
struct WorldEventNotice;
struct WorldSnapshot;

}

namespace rtype {

class VisualEffects {
private:
	static constexpr std::size_t MAX_EFFECTS = 128;
	static constexpr float PLAYER_WEAPON_OFFSET = 32.0F;
	static constexpr float OFFSCREEN_MARGIN = 96.0F;  // matches the server margin past which entities are removed
	static constexpr auto BURST_FRAME_TIME = std::chrono::milliseconds(55);
	static constexpr auto IMPACT_FRAME_TIME = std::chrono::milliseconds(40);
	static constexpr auto MOB_EXPLOSION_FRAME_TIME = std::chrono::milliseconds(25);
	static constexpr auto PLAYER_EXPLOSION_FRAME_TIME = std::chrono::milliseconds(70);
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

	enum class EffectKind : std::uint8_t { ShotBurst,
		HitSpark,
		MobExplosion,
		PlayerExplosion };

	struct VisualEffect {
		std::optional<std::uint32_t> entityId;
		std::optional<std::uint8_t> followPlayerId;
		EffectKind kind;
		sf::Vector2f position;
		LocalFirePredictor::TimePoint started;
	};

	static std::chrono::milliseconds effectDuration(EffectKind kind);
	void updatePositions(const std::optional<network::WorldSnapshot>& snapshot);
	void updateConnections(const std::optional<network::WorldSnapshot>& snapshot);
	void addEffect(std::optional<std::uint32_t> entityId, EffectKind kind, sf::Vector2f position, LocalFirePredictor::TimePoint now,
		std::optional<std::uint8_t> followPlayerId = std::nullopt);
	void drawEffect(sf::RenderWindow& window, VisualEffect& effect, LocalFirePredictor::TimePoint now);
	void updateEffectPosition(VisualEffect& effect);
	void handleEvent(const network::EntitySpawnEvent& spawn, LocalFirePredictor::TimePoint now);
	void handleEvent(const network::EntityDestroyEvent& destroy, LocalFirePredictor::TimePoint now);
	void handleEvent(const network::EntityDamageEvent& damage, LocalFirePredictor::TimePoint now);
	void handleEvent(const network::PlayerDeathEvent& death, LocalFirePredictor::TimePoint now);
	void removeHitSpark(std::uint32_t entityId);

	AssetManager& _assets;
	SoundPlayer& _sounds;
	LocalFirePredictor _firePredictor;
	std::optional<std::uint8_t> _localPlayerId;
	std::optional<std::uint32_t> _localPlayerEntityId;
	std::optional<std::uint64_t> _positionsTick;
	std::array<std::optional<std::uint32_t>, client_config::MAX_PLAYERS> _playerEntityIds;
	std::unordered_map<std::uint32_t, sf::Vector2f> _entityPositions;
	std::vector<VisualEffect> _effects;
	std::uint8_t _connectedMask = 0;

public:
	explicit VisualEffects(AssetManager& assets, SoundPlayer& sounds);
	void predictLocalFire(std::uint8_t buttons, std::optional<std::uint8_t> localPlayerId, const std::optional<network::WorldSnapshot>& snapshot);
	void display(sf::RenderWindow& window, const std::optional<network::WorldSnapshot>& snapshot,
		const std::vector<network::WorldEventNotice>& events);
};

}
