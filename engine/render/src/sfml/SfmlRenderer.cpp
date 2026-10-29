/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML graphics backend
*/

#include "engine/render/SfmlRenderer.hpp"

#include <cmath>
#include <filesystem>
#include <iostream>

#include "SfmlRendererImplementation.hpp"
#include "engine/render/GraphicsError.hpp"

namespace engine::render {

std::unordered_map<TextureId, sf::Texture>::iterator SfmlRenderer::Implementation::resolveTexture(
	TextureId textureId)
{
	return loadedTextures.find(textureId);
}

SfmlRenderer::Implementation::Implementation(const WindowConfig& config)
	: window(sf::VideoMode({config.width, config.height}), config.title)
{
}

/// Opens the window.
SfmlRenderer::SfmlRenderer(const WindowConfig& config) : _implementation(std::make_unique<Implementation>(config))
{
	_implementation->window.setFramerateLimit(config.frameRateLimit);
}

SfmlRenderer::~SfmlRenderer() = default;

bool SfmlRenderer::isOpen() const
{
	return _implementation->window.isOpen();
}

bool SfmlRenderer::hasFocus() const
{
	return _implementation->window.hasFocus();
}

void SfmlRenderer::close()
{
	_implementation->window.close();
}

/// The window closes itself when its close button is pressed.
void SfmlRenderer::handleEvent()
{
	while (const auto event = _implementation->window.pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			_implementation->window.close();
		}
	}
}

/// Loads a texture from file into the SFML texture cache and returns an ID.
TextureId SfmlRenderer::loadTextureFromFile(const std::string& filepath)
{
	const auto existingMapping = _implementation->pathToTextureId.find(filepath);
	if (existingMapping != _implementation->pathToTextureId.end()) {
		return existingMapping->second;
	}

	sf::Texture loadedTexture;
	if (!loadedTexture.loadFromFile(std::filesystem::path(filepath))) {
		_implementation->pathToTextureId.emplace(filepath, INVALID_TEXTURE_ID);
		return INVALID_TEXTURE_ID;
	}
	const TextureId assignedTextureId = _implementation->nextTextureId++;
	_implementation->loadedTextures.emplace(assignedTextureId, std::move(loadedTexture));
	_implementation->pathToTextureId.emplace(filepath, assignedTextureId);
	return assignedTextureId;
}

/// Loads a texture before the first frame, so a missing required file fails at startup
/// instead of being drawn as the fallback texture.
void SfmlRenderer::preload(std::string_view texture)
{
	if (loadTextureFromFile(std::string(texture)) == INVALID_TEXTURE_ID) {
		throw GraphicsError("Failed to preload required texture: " + std::string(texture));
	}
}

void SfmlRenderer::beginFrame()
{
	_implementation->window.clear(sf::Color::Black);
}

/// Draws the repeated background at the offset selected by the render system.
void SfmlRenderer::drawBackground(TextureId textureId, float offset)
{
	const auto textureIterator = _implementation->resolveTexture(textureId);
	if (textureIterator == _implementation->loadedTextures.end()) {
		return;
	}
	textureIterator->second.setRepeated(true);
	const sf::Texture& backgroundImage = textureIterator->second;
	const sf::Vector2u windowSize = _implementation->window.getSize();
	if (windowSize.x == 0 || windowSize.y == 0 || backgroundImage.getSize().x == 0) {
		return;
	}
	const float wrappedOffset = std::fmod(offset, static_cast<float>(backgroundImage.getSize().x));
	const int leftPosition = static_cast<int>(wrappedOffset);
	sf::Sprite backgroundSprite(backgroundImage,
		sf::IntRect({leftPosition, 0}, {static_cast<int>(windowSize.x) + 1, static_cast<int>(windowSize.y)}));
	backgroundSprite.setPosition({-(wrappedOffset - static_cast<float>(leftPosition)), 0.0F});
	_implementation->window.draw(backgroundSprite);
}

/// Translates a library independent sprite command into an SFML draw call.
void SfmlRenderer::drawSprite(const SpriteDraw& command)
{
	const auto textureIterator = _implementation->resolveTexture(command.textureId);
	if (textureIterator == _implementation->loadedTextures.end()) {
		return;
	}
	const sf::Texture& spriteTexture = textureIterator->second;
	const Rect& region = command.region;
	sf::Sprite renderedSprite(spriteTexture,
		sf::IntRect({region.position.x, region.position.y}, {region.size.x, region.size.y}));
	renderedSprite.setOrigin({command.originX, command.originY});
	renderedSprite.setScale({command.scale, command.scale});
	renderedSprite.setRotation(sf::degrees(command.rotation));
	renderedSprite.setPosition({command.x, command.y});
	_implementation->window.draw(renderedSprite);
}

void SfmlRenderer::endFrame()
{
	_implementation->window.display();
}

}
