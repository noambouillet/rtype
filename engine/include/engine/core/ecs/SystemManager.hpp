/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SystemManager
*/

#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

#include "engine/core/ecs/Entity.hpp"
#include "engine/core/ecs/System.hpp"
#include "engine/core/ecs/Types.hpp"

namespace engine::core {

class SystemManager {
public:
	SystemManager() = default;
	~SystemManager() = default;

	template <typename T>
	T& registerSystem();

	template <typename T>
	void setSignature(Signature signature);

	template <typename T>
	[[nodiscard]] T& getSystem();

	void entityDestroyed(Entity entity);
	void entitySignatureChanged(Entity entity, Signature entitySignature);

private:
	std::unordered_map<std::type_index, Signature> _signatures;
	std::unordered_map<std::type_index, std::unique_ptr<System>> _systems;
};

}

#include "engine/core/ecs/SystemManager.tpp"
