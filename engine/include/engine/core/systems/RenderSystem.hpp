/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Render system
*/

#pragma once

#include <string>

#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"
#include "engine/render/GraphicsBackend.hpp"

namespace engine::core {

class RenderSystem : public System {
public:
	RenderSystem();
	Signature signature(Coordinator& coordinator);
	void setBackground(std::string texture, float speed);
	void render(Coordinator& coordinator, render::IGraphicsBackend& backend, float deltaTime);

private:
	std::string _backgroundTexture;
	float _backgroundSpeed;
	float _backgroundOffset;
};

}
