/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML objects owned by SfmlRenderer, kept out of the public header
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>

#include "engine/render/GraphicsBackend.hpp"
#include "engine/render/SfmlRenderer.hpp"

namespace engine::render {

struct SfmlRenderer::Implementation {
	explicit Implementation(const WindowConfig& config);

	sf::RenderWindow window;
	sf::View worldView;
	sf::View screenView;
	std::unordered_map<TextureId, sf::Texture> loadedTextures;
	TextureId nextTextureId = 1;

	std::unordered_map<FontId, sf::Font> loadedFonts;
	FontId nextFontId = 1;

	std::unordered_map<TextureId, sf::Texture>::iterator resolveTexture(TextureId textureId);
	std::unordered_map<FontId, sf::Font>::iterator resolveFont(FontId fontId);
};

}
