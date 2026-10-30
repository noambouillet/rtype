/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Entity prefabs: one function per kind of game entity
*/

#include "Prefabs.hpp"

#include <cmath>

#include "GameConfig.hpp"
#include "engine/physics/Components.hpp"
#include "server/ecs/GameComponents.hpp"
#include "server/ecs/Systems/ReplicationSystem.hpp"

namespace rtype::prefabs {

/// Builds a box collider centred on the entity's position.
static engine::physics::Collider centredCollider(float width, float height, std::uint32_t tag, std::uint32_t mask)
{
	return engine::physics::Collider{.width = width, .height = height, .offsetX = -width / 2.0F, .offsetY = -height / 2.0F, .tag = tag, .mask = mask};
}

/// Tells whether one more replicated entity still fits in a snapshot; the RFC delays spawns past 64.
bool canSpawnReplicated(engine::core::Coordinator& coordinator)
{
	return coordinator.getSystem<ecs::ReplicationSystem>().entities.size() < config::MAX_REPLICATED_ENTITIES;
}

/// Creates a player's ship at its starting slot on the left side.
engine::core::Entity createPlayer(engine::core::Coordinator& coordinator, std::uint8_t playerId)
{
	const engine::core::Entity entity = coordinator.createEntity();
	const float startY = config::PLAYER_START_Y + (static_cast<float>(playerId) * config::PLAYER_START_SPACING);

	coordinator.addComponent(entity, engine::core::Position{.x = config::PLAYER_START_X, .y = startY});
	coordinator.addComponent(entity, engine::core::Velocity{});
	coordinator.addComponent(entity, ecs::Health{.maxHP = config::PLAYER_HEALTH, .hp = config::PLAYER_HEALTH});
	coordinator.addComponent(entity, ecs::PlayerControl{.playerId = playerId, .speed = config::PLAYER_SPEED});
	coordinator.addComponent(entity, ecs::Weapon{.cooldown = config::FIRE_COOLDOWN, .remaining = 0.0F, .firing = false});
	coordinator.addComponent(entity, centredCollider(config::PLAYER_WIDTH, config::PLAYER_HEIGHT,
										 ecs::layer::PLAYER, ecs::layer::ENEMY | ecs::layer::ENEMY_PROJECTILE));
	coordinator.addComponent(entity, ecs::Offscreen{ecs::OffscreenAction::Block});
	coordinator.addComponent(entity, ecs::Replicated{.type = game::EntityType::Player, .owner = playerId, .variant = playerId});
	return entity;
}

/// Creates a projectile at the nose of a player's ship, flying right until it hits an enemy or leaves the world.
/// The origin is taken by value: adding components may reallocate the storage a reference would point into.
engine::core::Entity createPlayerProjectile(engine::core::Coordinator& coordinator, std::uint8_t playerId, engine::core::Position origin)
{
	const engine::core::Entity entity = coordinator.createEntity();

	coordinator.addComponent(entity, engine::core::Position{.x = origin.x + (config::PLAYER_WIDTH / 2.0F), .y = origin.y});
	coordinator.addComponent(entity, engine::core::Velocity{.x = config::PROJECTILE_SPEED, .y = 0.0F});
	coordinator.addComponent(entity, ecs::Damage{config::PROJECTILE_DAMAGE});
	coordinator.addComponent(entity, centredCollider(config::PROJECTILE_WIDTH, config::PROJECTILE_HEIGHT,
										 ecs::layer::PLAYER_PROJECTILE, ecs::layer::ENEMY));
	coordinator.addComponent(entity, ecs::DestroyOnHit{});
	coordinator.addComponent(entity, ecs::Offscreen{ecs::OffscreenAction::Destroy});
	coordinator.addComponent(entity, ecs::Replicated{.type = game::EntityType::PlayerProjectile, .owner = playerId, .variant = 0});
	return entity;
}

/// Creates an enemy shot flying at the velocity aimed by WeaponSystem until it hits a player or leaves the world.
/// The origin is taken by value for the same reason as in createPlayerProjectile.
engine::core::Entity createEnemyProjectile(engine::core::Coordinator& coordinator, engine::core::Position origin, engine::core::Velocity velocity)
{
	const engine::core::Entity entity = coordinator.createEntity();

	coordinator.addComponent(entity, origin);
	coordinator.addComponent(entity, velocity);
	coordinator.addComponent(entity, ecs::Damage{config::ENEMY_PROJECTILE_DAMAGE});
	coordinator.addComponent(entity, centredCollider(config::ENEMY_PROJECTILE_WIDTH, config::ENEMY_PROJECTILE_HEIGHT,
										 ecs::layer::ENEMY_PROJECTILE, ecs::layer::PLAYER));
	coordinator.addComponent(entity, ecs::DestroyOnHit{});
	coordinator.addComponent(entity, ecs::Offscreen{ecs::OffscreenAction::Destroy});
	coordinator.addComponent(entity, ecs::Replicated{.type = game::EntityType::EnemyProjectile, .owner = game::NO_OWNER, .variant = 0});
	return entity;
}

/// Creates a charge-shot beam at the cannon of a hovering Scant, flying straight left until it hits a player or
/// leaves the world. The origin is taken by value for the same reason as in createPlayerProjectile.
engine::core::Entity createScantBeam(engine::core::Coordinator& coordinator, engine::core::Position origin)
{
	const engine::core::Entity entity = coordinator.createEntity();

	coordinator.addComponent(entity, engine::core::Position{.x = origin.x - ((config::SCANT_WIDTH + config::SCANT_BEAM_WIDTH) / 2.0F),
										 .y = origin.y + config::SCANT_CANNON_Y});
	coordinator.addComponent(entity, engine::core::Velocity{.x = -config::SCANT_BEAM_SPEED, .y = 0.0F});
	coordinator.addComponent(entity, ecs::Damage{config::SCANT_BEAM_DAMAGE});
	coordinator.addComponent(entity, centredCollider(config::SCANT_BEAM_WIDTH, config::SCANT_BEAM_HEIGHT,
										 ecs::layer::ENEMY_PROJECTILE, ecs::layer::PLAYER));
	coordinator.addComponent(entity, ecs::DestroyOnHit{});
	coordinator.addComponent(entity, ecs::Offscreen{ecs::OffscreenAction::Destroy});
	coordinator.addComponent(entity, ecs::Replicated{.type = game::EntityType::EnemyProjectile, .owner = game::NO_OWNER, .variant = static_cast<std::uint8_t>(game::ShotKind::Beam)});
	return entity;
}

/// Creates a Patapata just past the right edge, weaving around its spawn height; the phase sets where on its wave it starts,
/// so the members of a group do not rise and fall together. SineMotionSystem makes it weave left.
/// It fires at the players a first time after spawning, then every PATAPATA_FIRE_COOLDOWN seconds.
engine::core::Entity createPatapata(engine::core::Coordinator& coordinator, EnemySpawn spawn, float phase)
{
	const engine::core::Entity entity = coordinator.createEntity();
	const float startY = spawn.y + (config::PATAPATA_AMPLITUDE * std::sin(phase));

	coordinator.addComponent(entity, engine::core::Position{.x = config::WORLD_WIDTH + (config::PATAPATA_WIDTH / 2.0F), .y = startY});
	coordinator.addComponent(entity, engine::core::Velocity{});
	coordinator.addComponent(entity, ecs::SineMotion{.speed = config::PATAPATA_SPEED, .amplitude = config::PATAPATA_AMPLITUDE, .frequency = config::PATAPATA_FREQUENCY, .phase = phase, .elapsed = 0.0F});
	coordinator.addComponent(entity, ecs::Weapon{.cooldown = config::PATAPATA_FIRE_COOLDOWN, .remaining = spawn.firstShot, .firing = true, .shot = game::ShotKind::Aimed});
	coordinator.addComponent(entity, ecs::Health{.maxHP = config::PATAPATA_HEALTH, .hp = config::PATAPATA_HEALTH});
	coordinator.addComponent(entity, ecs::Damage{config::ENEMY_CONTACT_DAMAGE});
	coordinator.addComponent(entity, centredCollider(config::PATAPATA_WIDTH, config::PATAPATA_HEIGHT,
										 ecs::layer::ENEMY, ecs::layer::PLAYER | ecs::layer::PLAYER_PROJECTILE));
	coordinator.addComponent(entity, ecs::Offscreen{ecs::OffscreenAction::Destroy});
	coordinator.addComponent(entity, ecs::Replicated{.type = game::EntityType::Enemy, .owner = game::NO_OWNER, .variant = static_cast<std::uint8_t>(game::EnemyKind::Patapata)});
	return entity;
}

/// Creates a Bug just past the right edge at its spawn height; SineMotionSystem flies it along the arch.
/// It fires at the players a first time after spawning, then every BUG_FIRE_COOLDOWN seconds.
engine::core::Entity createBug(engine::core::Coordinator& coordinator, EnemySpawn spawn)
{
	const engine::core::Entity entity = coordinator.createEntity();

	coordinator.addComponent(entity, engine::core::Position{.x = config::WORLD_WIDTH + (config::BUG_WIDTH / 2.0F), .y = spawn.y});
	coordinator.addComponent(entity, engine::core::Velocity{});
	coordinator.addComponent(entity, ecs::SineMotion{.speed = config::BUG_SPEED, .amplitude = -config::BUG_ARC_HEIGHT, .frequency = config::BUG_FREQUENCY, .phase = 0.0F, .elapsed = 0.0F});
	coordinator.addComponent(entity, ecs::Weapon{.cooldown = config::BUG_FIRE_COOLDOWN, .remaining = spawn.firstShot, .firing = true, .shot = game::ShotKind::Aimed});
	coordinator.addComponent(entity, ecs::Health{.maxHP = config::BUG_HEALTH, .hp = config::BUG_HEALTH});
	coordinator.addComponent(entity, ecs::Damage{config::ENEMY_CONTACT_DAMAGE});
	coordinator.addComponent(entity, centredCollider(config::BUG_WIDTH, config::BUG_HEIGHT,
										 ecs::layer::ENEMY, ecs::layer::PLAYER | ecs::layer::PLAYER_PROJECTILE));
	coordinator.addComponent(entity, ecs::Offscreen{ecs::OffscreenAction::Destroy});
	coordinator.addComponent(entity, ecs::Replicated{.type = game::EntityType::Enemy, .owner = game::NO_OWNER, .variant = static_cast<std::uint8_t>(game::EnemyKind::Bug)});
	return entity;
}

/// Creates a Bink just past the right edge, standing on the ground and walking left at a constant speed.
engine::core::Entity createBink(engine::core::Coordinator& coordinator, float firstShot)
{
	const engine::core::Entity entity = coordinator.createEntity();
	coordinator.addComponent(entity, engine::core::Position{.x = config::WORLD_WIDTH + (config::BINK_WIDTH / 2.0F),
										 .y = config::GROUND_Y - (config::BINK_HEIGHT / 2.0F)});
	coordinator.addComponent(entity, engine::core::Velocity{.x = -config::BINK_SPEED, .y = 0.0F});
	coordinator.addComponent(entity, ecs::Weapon{.cooldown = config::BINK_FIRE_COOLDOWN, .remaining = firstShot, .firing = true, .shot = game::ShotKind::Aimed});
	coordinator.addComponent(entity, ecs::Health{.maxHP = config::BINK_HEALTH, .hp = config::BINK_HEALTH});
	coordinator.addComponent(entity, ecs::Damage{config::ENEMY_CONTACT_DAMAGE});
	coordinator.addComponent(entity, centredCollider(config::BINK_WIDTH, config::BINK_HEIGHT,
										 ecs::layer::ENEMY, ecs::layer::PLAYER | ecs::layer::PLAYER_PROJECTILE));
	coordinator.addComponent(entity, ecs::Offscreen{ecs::OffscreenAction::Destroy});
	coordinator.addComponent(entity, ecs::Replicated{.type = game::EntityType::Enemy, .owner = game::NO_OWNER, .variant = static_cast<std::uint8_t>(game::EnemyKind::Bink)});
	return entity;
}

/// Creates a Scant just past the right edge at its spawn height; HoverMotionSystem flies it in, keeps it level with the
/// closest player, then sends it off.
engine::core::Entity createScant(engine::core::Coordinator& coordinator, EnemySpawn spawn)
{
	const engine::core::Entity entity = coordinator.createEntity();

	coordinator.addComponent(entity, engine::core::Position{.x = config::WORLD_WIDTH + (config::SCANT_WIDTH / 2.0F), .y = spawn.y});
	coordinator.addComponent(entity, engine::core::Velocity{});
	coordinator.addComponent(entity, ecs::HoverMotion{.speed = config::SCANT_SPEED, .holdX = config::SCANT_HOLD_X, .trackSpeed = config::SCANT_TRACK_SPEED, .stayTime = config::SCANT_STAY_TIME, .chargeTime = config::SCANT_CHARGE_TIME, .elapsed = 0.0F});
	coordinator.addComponent(entity, ecs::Weapon{.cooldown = config::SCANT_FIRE_COOLDOWN, .remaining = spawn.firstShot, .firing = true, .shot = game::ShotKind::Beam});
	coordinator.addComponent(entity, ecs::Health{.maxHP = config::SCANT_HEALTH, .hp = config::SCANT_HEALTH});
	coordinator.addComponent(entity, ecs::Damage{config::ENEMY_CONTACT_DAMAGE});
	coordinator.addComponent(entity, centredCollider(config::SCANT_WIDTH, config::SCANT_HEIGHT,
										 ecs::layer::ENEMY, ecs::layer::PLAYER | ecs::layer::PLAYER_PROJECTILE));
	coordinator.addComponent(entity, ecs::Offscreen{ecs::OffscreenAction::Destroy});
	coordinator.addComponent(entity, ecs::Replicated{.type = game::EntityType::Enemy, .owner = game::NO_OWNER, .variant = static_cast<std::uint8_t>(game::EnemyKind::Scant)});
	return entity;
}

/// Creates an invisible (not replicated) entity that sends a group of enemies around a height, the first one next
/// tick: Bugs all start at that height so they trace one shared path, Patapata are scattered so each weaves on its
/// own. Binks ignore the height: they always walk on the ground.
engine::core::Entity createFormation(engine::core::Coordinator& coordinator, game::EnemyKind kind, float y, int count)
{
	const engine::core::Entity entity = coordinator.createEntity();
	const float delay = kind == game::EnemyKind::Bug ? config::BUG_DELAY : config::PATAPATA_DELAY;
	const float spread = kind == game::EnemyKind::Patapata ? config::PATAPATA_SPREAD : 0.0F;

	coordinator.addComponent(entity, ecs::FormationSpawner{.kind = kind, .remaining = count, .delay = delay, .timer = 0.0F, .y = y, .spread = spread});
	return entity;
}

/// Creates an invisible (not replicated) entity that sends a wave of enemies every WAVE_INTERVAL seconds.
engine::core::Entity createWaveSpawner(engine::core::Coordinator& coordinator)
{
	const engine::core::Entity entity = coordinator.createEntity();

	coordinator.addComponent(entity, ecs::WaveSpawner{.interval = config::WAVE_INTERVAL, .timer = config::WAVE_INTERVAL});
	return entity;
}

}
