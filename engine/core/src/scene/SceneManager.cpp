/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Stack of scenes
*/

#include "engine/core/scene/SceneManager.hpp"

#include <utility>

namespace engine::core {

SceneManager::SceneManager() : _scenes(), _pending()
{
}

/// Lets every scene left on the stack run its exit code, from the top one down.
SceneManager::~SceneManager()
{
	while (!_scenes.empty()) {
		popNow();
	}
}

/// Puts a scene over the current one, which keeps running without input. Applied at the end of the next update.
void SceneManager::push(std::unique_ptr<Scene> scene)
{
	_pending.push_back({.kind = OperationKind::Push, .scene = std::move(scene)});
}

/// Removes the top scene. Applied at the end of the next update.
void SceneManager::pop()
{
	_pending.push_back({.kind = OperationKind::Pop, .scene = nullptr});
}

/// Swaps the top scene for another one. Applied at the end of the next update.
void SceneManager::replace(std::unique_ptr<Scene> scene)
{
	_pending.push_back({.kind = OperationKind::Replace, .scene = std::move(scene)});
}

/// Removes every scene, so the game can stop once the stack is empty. Applied at the end of the next update.
void SceneManager::clear()
{
	_pending.push_back({.kind = OperationKind::Clear, .scene = nullptr});
}

/// Gives the input to the top scene only, then updates every scene from the bottom up.
void SceneManager::update(float deltaTime)
{
	applyPending();
	if (!_scenes.empty()) {
		_scenes.back()->handleInput();
	}
	for (const auto& scene : _scenes) {
		scene->update(deltaTime);
	}
	applyPending();
}

/// Draws every scene from the bottom up, so the top one is drawn over the others.
void SceneManager::render()
{
	for (const auto& scene : _scenes) {
		scene->render();
	}
}

/// Tells whether no scene is left, counting the ones asked for but not applied yet.
bool SceneManager::empty() const
{
	return _scenes.empty() && _pending.empty();
}

/// Runs the operations in the order they were asked. A scene may ask for more while entering or exiting.
void SceneManager::applyPending()
{
	while (!_pending.empty()) {
		std::vector<Operation> operations = std::move(_pending);
		_pending.clear();
		for (Operation& operation : operations) {
			switch (operation.kind) {
				case OperationKind::Push:
					if (operation.scene && !_scenes.empty()) {
						_scenes.back()->onPause();
					}
					pushNow(std::move(operation.scene));
					break;
				case OperationKind::Pop:
					popNow();
					if (!_scenes.empty()) {
						_scenes.back()->onResume();
					}
					break;
				case OperationKind::Replace:
					popNow();
					pushNow(std::move(operation.scene));
					break;
				case OperationKind::Clear:
					while (!_scenes.empty()) {
						popNow();
					}
					break;
			}
		}
	}
}

/// A replaced scene takes the place of the old one, so the scene below it stays paused.
void SceneManager::pushNow(std::unique_ptr<Scene> scene)
{
	if (!scene) {
		return;
	}
	_scenes.push_back(std::move(scene));
	_scenes.back()->onEnter();
}

void SceneManager::popNow()
{
	if (_scenes.empty()) {
		return;
	}
	_scenes.back()->onExit();
	_scenes.pop_back();
}

}
