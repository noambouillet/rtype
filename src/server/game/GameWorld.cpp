/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Server game world: owns the ECS and runs one gameplay tick
*/

#include "GameWorld.hpp"

#include "CollisionSystem.hpp"
#include "Components.hpp"
#include "DamageSystem.hpp"
#include "DestroySystem.hpp"
#include "MovementSystem.hpp"
#include "server/ecs/GameComponents.hpp"
#include "server/ecs/Systems/EnemySpawnSystem.hpp"
#include "server/ecs/Systems/FormationSystem.hpp"
#include "server/ecs/Systems/HitSystem.hpp"
#include "server/ecs/Systems/HoverMotionSystem.hpp"
#include "server/ecs/Systems/OffscreenSystem.hpp"
#include "server/ecs/Systems/PlayerControlSystem.hpp"
#include "server/ecs/Systems/SineMotionSystem.hpp"
#include "server/ecs/Systems/WeaponSystem.hpp"
#include "server/game/Prefabs.hpp"

namespace rtype {

/// Registers T and gives it the signature its own `signature()` describes.
template <typename T>
static void registerSystem(ecs::Coordinator& coordinator)
{
	auto& system = coordinator.registerSystem<T>();
	coordinator.setSystemSignature<T>(system.signature(coordinator));
}

/// Creates the world: components, then systems, then the entities that exist from the start.
GameWorld::GameWorld() : _coordinator{}, _playerEntities{}
{
	registerComponents();
	registerSystems();
	prefabs::createWaveSpawner(_coordinator);
}

/// Registers every component type; systems need them to build their signatures.
void GameWorld::registerComponents()
{
	_coordinator.registerComponent<ecs::Position>();
	_coordinator.registerComponent<ecs::Velocity>();
	_coordinator.registerComponent<ecs::Health>();
	_coordinator.registerComponent<ecs::Damage>();
	_coordinator.registerComponent<ecs::Destroy>();
	_coordinator.registerComponent<ecs::Collider>();
	_coordinator.registerComponent<ecs::Replicated>();
	_coordinator.registerComponent<ecs::PlayerControl>();
	_coordinator.registerComponent<ecs::Weapon>();
	_coordinator.registerComponent<ecs::WaveSpawner>();
	_coordinator.registerComponent<ecs::Offscreen>();
	_coordinator.registerComponent<ecs::DestroyOnHit>();
	_coordinator.registerComponent<ecs::SineMotion>();
	_coordinator.registerComponent<ecs::FormationSpawner>();
	_coordinator.registerComponent<ecs::HoverMotion>();
}

/// Registers every system; it must happen before any entity exists so each system sees them all.
void GameWorld::registerSystems()
{
	registerSystem<ecs::PlayerControlSystem>(_coordinator);
	registerSystem<ecs::WeaponSystem>(_coordinator);
	registerSystem<ecs::EnemySpawnSystem>(_coordinator);
	registerSystem<ecs::FormationSystem>(_coordinator);
	registerSystem<ecs::SineMotionSystem>(_coordinator);
	registerSystem<ecs::HoverMotionSystem>(_coordinator);
	registerSystem<ecs::MovementSystem>(_coordinator);
	registerSystem<ecs::CollisionSystem>(_coordinator);
	registerSystem<ecs::DamageSystem>(_coordinator);
	registerSystem<ecs::HitSystem>(_coordinator);
	registerSystem<ecs::OffscreenSystem>(_coordinator);
	registerSystem<ecs::DestroySystem>(_coordinator);
	registerSystem<ecs::ReplicationSystem>(_coordinator);
}

/// Advances one world tick and returns its gameplay events in simulation order.
std::vector<WorldEvent> GameWorld::update(const Input& input, float deltaTime)
{
	WorldEventCollector events;
	syncPlayerEntities(input, events);
	runSystems(input, deltaTime, events);
	return events.take();
}

/// Runs systems in dependency order, recording changes before DestroySystem removes their components.
void GameWorld::runSystems(const Input& input, float deltaTime, WorldEventCollector& events)
{
	_coordinator.getSystem<ecs::PlayerControlSystem>().update(_coordinator, input.buttons);
	const std::vector<ecs::Entity> projectiles = _coordinator.getSystem<ecs::WeaponSystem>().update(_coordinator, deltaTime);
	for (const ecs::Entity& projectile : projectiles) {
		events.recordSpawn(_coordinator, projectile);
	}
	_coordinator.getSystem<ecs::EnemySpawnSystem>().update(_coordinator, deltaTime, input.connectedMask != 0);
	const std::vector<ecs::Entity> enemies = _coordinator.getSystem<ecs::FormationSystem>().update(_coordinator, deltaTime);
	for (const ecs::Entity& enemy : enemies) {
		events.recordSpawn(_coordinator, enemy);
	}
	_coordinator.getSystem<ecs::SineMotionSystem>().update(_coordinator, deltaTime);
	_coordinator.getSystem<ecs::HoverMotionSystem>().update(_coordinator, deltaTime);
	_coordinator.getSystem<ecs::MovementSystem>().update(_coordinator, deltaTime);
	_coordinator.getSystem<ecs::CollisionSystem>().update(_coordinator);
	const std::vector<ecs::CollisionPair>& collisions = _coordinator.getSystem<ecs::CollisionSystem>().collisions();
	const std::vector<ecs::CollisionPair> accepted = _coordinator.getSystem<ecs::HitSystem>().update(_coordinator, collisions);
	const std::vector<ecs::HealthChange> changes = _coordinator.getSystem<ecs::DamageSystem>().update(_coordinator, accepted);
	for (const ecs::HealthChange& change : changes) {
		if (change.remainingHealth > 0) {
			events.recordDamage(_coordinator, change.entity);
		}
	}
	_coordinator.getSystem<ecs::OffscreenSystem>().update(_coordinator, config::WORLD_WIDTH, config::WORLD_HEIGHT, config::OFFSCREEN_MARGIN);
	for (const ecs::Entity& removed : _coordinator.getSystem<ecs::DestroySystem>().entities) {
		events.recordRemoval(_coordinator, removed);
	}
	_coordinator.getSystem<ecs::DestroySystem>().update(_coordinator);
}

/// Synchronizes player ships with sessions and reports newly spawned ships.
/// A disconnect removes its ship without reporting a gameplay death. Reconnecting respawns a dead player.
void GameWorld::syncPlayerEntities(const Input& input, WorldEventCollector& events)
{
	for (std::size_t playerId = 0; playerId < MAX_PLAYERS; playerId++) {
		const auto playerMask = static_cast<std::uint8_t>(1 << playerId);
		const bool connected = (input.connectedMask & playerMask) != 0;

		if (connected && !_playerEntities[playerId]) {
			const ecs::Entity player = prefabs::createPlayer(_coordinator, static_cast<std::uint8_t>(playerId));
			_playerEntities[playerId] = player;
			events.recordSpawn(_coordinator, player);
		} else if (!connected) {
			auto& player = _playerEntities[playerId];
			if (player) {
				_coordinator.destroyEntity(player.value());
				player.reset();
			}
		}
	}
}

/// Returns the bit field of players whose ship is still alive.
std::uint8_t GameWorld::aliveMask() const
{
	std::uint8_t mask = 0;
	for (std::size_t playerId = 0; playerId < MAX_PLAYERS; playerId++) {
		const auto& player = _playerEntities[playerId];
		if (player && _coordinator.isAlive(player.value())) {
			mask |= static_cast<std::uint8_t>(1 << playerId);
		}
	}
	return mask;
}

/// Lists authoritative visual state for the network service to encode.
std::vector<GameWorld::EntityState> GameWorld::replicatedEntities()
{
	return _coordinator.getSystem<ecs::ReplicationSystem>().collect(_coordinator);
}

}
