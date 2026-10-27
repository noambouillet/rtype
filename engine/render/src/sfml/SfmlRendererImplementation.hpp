/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML objects owned by SfmlRenderer, kept out of the public header
*/

#pragma once

#include <SFML/Graphics.hpp>

#include "AssetManager.hpp"
#include "engine/render/SfmlRenderer.hpp"

namespace engine::render {

struct SfmlRenderer::Implementation {
	explicit Implementation(const WindowConfig& config);

	AssetManager assets;
	sf::RenderWindow window;
};

}
