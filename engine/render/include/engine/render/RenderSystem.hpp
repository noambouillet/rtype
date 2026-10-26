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

namespace engine::render {

class RenderSystem : public core::System {
public:
	RenderSystem();
	core::Signature signature(core::Coordinator& coordinator);
	void setBackground(std::string texture, float speed);
	void render(core::Coordinator& coordinator, IGraphicsBackend& backend, float deltaTime);

private:
	std::string _backgroundTexture;
	float _backgroundSpeed;
	float _backgroundOffset;
};

}
