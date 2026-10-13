/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Temporary client-side visual effects
*/

#include "VisualEffects.hpp"

#include <algorithm>
#include <bit>
#include <span>
#include <string_view>
#include <variant>

#include "SpriteRegions.hpp"
#include "client/network/GameClient.hpp"

namespace rtype {

/// Draws one frame from a spritesheet animation at a world position.
static void drawAnimation(sf::RenderWindow& window, AssetManager& assets, std::string_view sheet,
	std::span<const sf::IntRect> frames, sf::Vector2f position, std::chrono::milliseconds age,
	std::chrono::milliseconds frameTime)
{
	const auto index = static_cast<std::size_t>(age.count() / frameTime.count());
	if (index >= frames.size()) {
		return;
	}
	const sf::IntRect& frame = frames[index];
	sf::Sprite sprite = assets.sprite(sheet, frame);
	sprite.setOrigin({static_cast<float>(frame.size.x) / 2.0F, static_cast<float>(frame.size.y) / 2.0F});
	sprite.setScale({2.0F, 2.0F});
	sprite.setPosition(position);
	window.draw(sprite);
}

VisualEffects::VisualEffects(AssetManager& assets, SoundPlayer& sounds) : _assets(assets), _sounds(sounds)
{
	_playerEntityIds.fill(std::nullopt);
}

/// Starts a local weapon burst at the same cadence as the server's shots.
void VisualEffects::predictLocalFire(std::uint8_t buttons, std::optional<std::uint8_t> localPlayerId,
	const std::optional<network::WorldSnapshot>& snapshot)
{
	_localPlayerId = localPlayerId;
	_localPlayerEntityId.reset();
	std::optional<sf::Vector2f> weaponPosition;
	if (localPlayerId && snapshot && *localPlayerId < client_config::MAX_PLAYERS && (snapshot->aliveMask & (1 << *localPlayerId)) != 0) {
		for (const network::WorldEntity& entity : snapshot->entities) {
			if (entity.entityType == network::EntityType::Player && entity.ownerPlayerId == *localPlayerId) {
				_localPlayerEntityId = entity.entityId;
				weaponPosition = sf::Vector2f{entity.x + PLAYER_WEAPON_OFFSET, entity.y};
				break;
			}
		}
	}
	const auto now = std::chrono::steady_clock::now();
	if (_firePredictor.update((buttons & network::BUTTON_FIRE) != 0, weaponPosition.has_value(), now) && weaponPosition) {
		addEffect(std::nullopt, EffectKind::ShotBurst, *weaponPosition, now, localPlayerId);
	}
}

/// Returns the lifetime of one temporary animation.
std::chrono::milliseconds VisualEffects::effectDuration(EffectKind kind)
{
	switch (kind) {
		case EffectKind::ShotBurst:
			return BURST_FRAME_TIME * sprites::SHOT_BURST_FRAMES.size();
		case EffectKind::HitSpark:
			return IMPACT_FRAME_TIME * sprites::HIT_SPARK_FRAMES.size();
		case EffectKind::MobExplosion:
			return MOB_EXPLOSION_FRAME_TIME * sprites::MOB_EXPLOSION_FRAMES.size();
		case EffectKind::PlayerExplosion:
			return PLAYER_EXPLOSION_FRAME_TIME * sprites::PLAYER_EXPLOSION_FRAMES.size();
	}
	return std::chrono::milliseconds(0);
}

/// Keeps current entity positions and ship IDs from the newest snapshot.
void VisualEffects::updatePositions(const std::optional<network::WorldSnapshot>& snapshot)
{
	if (!snapshot || (_positionsTick && *_positionsTick == snapshot->tick)) {
		return;
	}
	_entityPositions.clear();
	_playerEntityIds.fill(std::nullopt);
	for (const network::WorldEntity& entity : snapshot->entities) {
		_entityPositions.insert_or_assign(entity.entityId, sf::Vector2f{entity.x, entity.y});
		if (entity.entityType == network::EntityType::Player && entity.ownerPlayerId < client_config::MAX_PLAYERS) {
			_playerEntityIds[entity.ownerPlayerId] = entity.entityId;
		}
	}
	_positionsTick = snapshot->tick;
}

/// Starts one bounded animation per entity and effect kind.
void VisualEffects::addEffect(std::optional<std::uint32_t> entityId, EffectKind kind, sf::Vector2f position,
	LocalFirePredictor::TimePoint now, std::optional<std::uint8_t> followPlayerId)
{
	for (VisualEffect& effect : _effects) {
		if (entityId && effect.entityId == entityId && effect.kind == kind) {
			effect.position = position;
			return;
		}
	}
	if (_effects.size() == MAX_EFFECTS) {
		_effects.erase(_effects.begin());
	}
	_effects.push_back(VisualEffect{.entityId = entityId,
		.followPlayerId = followPlayerId,
		.kind = kind,
		.position = position,
		.started = now});
	if (kind == EffectKind::ShotBurst) {
		_sounds.playSound(PLAYER_SHOT_SOUND, PLAYER_SHOT_VOLUME);
	} else if (kind == EffectKind::MobExplosion) {
		_sounds.playSound(ENEMY_DEATH_SOUND, ENEMY_DEATH_VOLUME);
	} else if (kind == EffectKind::PlayerExplosion) {
		_sounds.playSound(PLAYER_DEATH_SOUND, PLAYER_DEATH_VOLUME);
	}
}

/// Moves an effect with its known ship or damaged entity while it is visible.
void VisualEffects::updateEffectPosition(VisualEffect& effect)
{
	if (effect.kind == EffectKind::ShotBurst && effect.followPlayerId && *effect.followPlayerId < client_config::MAX_PLAYERS) {
		const auto shipId = _playerEntityIds[*effect.followPlayerId];
		if (shipId && _entityPositions.contains(*shipId)) {
			const auto shipPosition = _entityPositions.at(*shipId);
			effect.position = {shipPosition.x + PLAYER_WEAPON_OFFSET, shipPosition.y};
		}
	}
	if (effect.kind == EffectKind::HitSpark && effect.entityId && _entityPositions.contains(*effect.entityId)) {
		effect.position = _entityPositions.at(*effect.entityId);
	}
}

/// Draws the current sprite frame of one effect.
void VisualEffects::drawEffect(sf::RenderWindow& window, VisualEffect& effect, LocalFirePredictor::TimePoint now)
{
	updateEffectPosition(effect);
	const auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - effect.started);
	switch (effect.kind) {
		case EffectKind::ShotBurst:
			drawAnimation(window, _assets, sprites::SHOT_SHEET, sprites::SHOT_BURST_FRAMES, effect.position, age, BURST_FRAME_TIME);
			break;
		case EffectKind::HitSpark:
			drawAnimation(window, _assets, sprites::SHOT_SHEET, sprites::HIT_SPARK_FRAMES, effect.position, age, IMPACT_FRAME_TIME);
			break;
		case EffectKind::MobExplosion:
			drawAnimation(window, _assets, sprites::EXPLOSION_SHEET, sprites::MOB_EXPLOSION_FRAMES, effect.position, age, MOB_EXPLOSION_FRAME_TIME);
			break;
		case EffectKind::PlayerExplosion:
			drawAnimation(window, _assets, sprites::EXPLOSION_SHEET, sprites::PLAYER_EXPLOSION_FRAMES, effect.position, age, PLAYER_EXPLOSION_FRAME_TIME);
			break;
	}
}

/// Registers spawned positions and follows the owner of each remote shot.
void VisualEffects::handleEvent(const network::EntitySpawnEvent& spawn, LocalFirePredictor::TimePoint now)
{
	const auto& entity = spawn.entity;
	const sf::Vector2f position{entity.x, entity.y};
	_entityPositions.insert_or_assign(entity.entityId, position);
	if (entity.entityType == network::EntityType::Player && entity.ownerPlayerId < client_config::MAX_PLAYERS) {
		_playerEntityIds[entity.ownerPlayerId] = entity.entityId;
	}
	if (entity.entityType == network::EntityType::EnemyProjectile) {
		_sounds.playSound(ENEMY_SHOT_SOUND, ENEMY_SHOT_VOLUME);
	}
	if (entity.entityType != network::EntityType::PlayerProjectile) {
		return;
	}
	if (_localPlayerId && _localPlayerEntityId && entity.ownerPlayerId == *_localPlayerId) {
		return;
	}
	addEffect(entity.entityId, EffectKind::ShotBurst, position, now, entity.ownerPlayerId);
}

/// Removes an entity's hit spark after its destruction or death.
void VisualEffects::removeHitSpark(std::uint32_t entityId)
{
	std::erase_if(_effects, [entityId](const VisualEffect& effect) {
		return effect.entityId == entityId && effect.kind == EffectKind::HitSpark;
	});
}

/// Starts an enemy explosion when it is destroyed inside the world, not when it leaves it.
void VisualEffects::handleEvent(const network::EntityDestroyEvent& destroy, LocalFirePredictor::TimePoint now)
{
	_entityPositions.erase(destroy.entityId);
	removeHitSpark(destroy.entityId);
	const bool offscreen = destroy.x < -OFFSCREEN_MARGIN || destroy.x > static_cast<float>(client_config::WINDOW_WIDTH) + OFFSCREEN_MARGIN ||
						   destroy.y < -OFFSCREEN_MARGIN || destroy.y > static_cast<float>(client_config::WINDOW_HEIGHT) + OFFSCREEN_MARGIN;
	if (destroy.kind == network::EntityType::Enemy && !offscreen) {
		addEffect(destroy.entityId, EffectKind::MobExplosion, {destroy.x, destroy.y}, now);
	}
}

/// Places damage effects at the latest known entity position.
void VisualEffects::handleEvent(const network::EntityDamageEvent& damage, LocalFirePredictor::TimePoint now)
{
	const auto position = _entityPositions.find(damage.entityId);
	if (position != _entityPositions.end()) {
		addEffect(damage.entityId, EffectKind::HitSpark, position->second, now);
	}
}

/// Starts the player's death animation and stops following the lost ship.
void VisualEffects::handleEvent(const network::PlayerDeathEvent& death, LocalFirePredictor::TimePoint now)
{
	_entityPositions.erase(death.entityId);
	if (death.playerId < client_config::MAX_PLAYERS && _playerEntityIds[death.playerId] == death.entityId) {
		_playerEntityIds[death.playerId].reset();
	}
	removeHitSpark(death.entityId);
	addEffect(death.entityId, EffectKind::PlayerExplosion, {death.x, death.y}, now);
}

/// Plays one sound per player gained or lost by the connected players mask.
void VisualEffects::updateConnections(const std::optional<network::WorldSnapshot>& snapshot)
{
	if (!snapshot) {
		return;
	}
	const std::uint8_t connected = snapshot->connectedMask;
	const int joined = std::popcount(static_cast<std::uint8_t>(connected & ~_connectedMask));
	const int left = std::popcount(static_cast<std::uint8_t>(_connectedMask & ~connected));
	for (int player = 0; player < joined; ++player) {
		_sounds.playSound(PLAYER_JOIN_SOUND, CONNECTION_VOLUME);
	}
	for (int player = 0; player < left; ++player) {
		_sounds.playSound(PLAYER_LEAVE_SOUND, CONNECTION_VOLUME);
	}
	_connectedMask = connected;
}

/// Converts gameplay notices into temporary animations and draws their frames.
void VisualEffects::display(sf::RenderWindow& window, const std::optional<network::WorldSnapshot>& snapshot,
	const std::vector<network::WorldEventNotice>& events)
{
	updatePositions(snapshot);
	updateConnections(snapshot);
	const auto now = std::chrono::steady_clock::now();
	std::erase_if(_effects, [now](const VisualEffect& effect) { return now - effect.started >= effectDuration(effect.kind); });
	for (const network::WorldEventNotice& notice : events) {
		std::visit([this, now](const auto& event) { handleEvent(event, now); }, notice.data);
	}
	for (VisualEffect& effect : _effects) {
		drawEffect(window, effect, now);
	}
}

}
