/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Stack of scenes
*/

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "engine/core/scene/Scene.hpp"

namespace engine::core {

class SceneManager {
public:
	SceneManager();
	~SceneManager();

	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

	void push(std::unique_ptr<Scene> scene);
	void pop();
	void replace(std::unique_ptr<Scene> scene);
	void clear();

	void update(float deltaTime);
	void render();
	[[nodiscard]] bool empty() const;

private:
	enum class OperationKind : std::uint8_t {
		Push,
		Pop,
		Replace,
		Clear,
	};

	struct Operation {
		OperationKind kind;
		std::unique_ptr<Scene> scene;
	};

	void applyPending();
	void pushNow(std::unique_ptr<Scene> scene);
	void popNow();

	std::vector<std::unique_ptr<Scene>> _scenes;
	std::vector<Operation> _pending;
};

}
