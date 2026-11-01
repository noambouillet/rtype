/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** One screen of a game, run by the SceneManager
*/

#pragma once

namespace engine::core {

class Scene {
public:
	virtual ~Scene() = default;

	virtual void onEnter();
	virtual void onExit();
	virtual void onPause();
	virtual void onResume();
	virtual void handleInput() = 0;
	virtual void update(float deltaTime) = 0;
	virtual void render() = 0;
};

}
