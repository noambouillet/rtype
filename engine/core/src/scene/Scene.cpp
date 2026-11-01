/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** One screen of a game, run by the SceneManager
*/

#include "engine/core/scene/Scene.hpp"

namespace engine::core {

/// Called once the scene is on the stack, before its first update.
void Scene::onEnter()
{
}

/// Called before the scene is removed from the stack and destroyed.
void Scene::onExit()
{
}

/// Called when another scene is pushed over this one: it keeps being updated and drawn, but gets no input.
void Scene::onPause()
{
}

/// Called when the scene above this one is removed, so this one gets the input again.
void Scene::onResume()
{
}

}
