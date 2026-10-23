/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Coordinator
*/

#pragma once
#include <cstddef>

#include "engine/core/ecs/ComponentManager.hpp"
#include "engine/core/ecs/Entity.hpp"
#include "engine/core/ecs/EntityManager.hpp"
#include "engine/core/ecs/SystemManager.hpp"

namespace engine::core {
class Coordinator {
public:
	explicit Coordinator(std::size_t maxEntities = EntityManager::DEFAULT_MAX_ENTITIES);
	~Coordinator() = default;
	Entity createEntity();
	void destroyEntity(Entity e);
	[[nodiscard]] bool isAlive(Entity e) const;

	template <typename T>
	void registerComponent();
	template <typename T>
	void addComponent(Entity e, T comp);
	template <typename T>
	void removeComponent(Entity e);
	template <typename T>
	bool hasComponent(Entity e);
	template <typename T>
	T& getComponent(Entity e);
	template <typename T>
	ComponentType getComponentType();

	template <typename T>
	T& registerSystem();
	template <typename T>
	void setSystemSignature(Signature s);
	template <typename T>
	T& getSystem();

private:
	EntityManager _entitiesManager;
	ComponentManager _componentsManager;
	SystemManager _systemsManager;
};
}

#include "engine/core/ecs/CoordinatorComponent.tpp"
#include "engine/core/ecs/CoordinatorSystem.tpp"
