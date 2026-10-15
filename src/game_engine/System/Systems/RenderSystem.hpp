/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Render system
*/

#pragma once

#include <string>

#include "Coordinator.hpp"
#include "System.hpp"
#include "game_engine/Graphics/GraphicsBackend.hpp"

namespace rtype::ecs {

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
