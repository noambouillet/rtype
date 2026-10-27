/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML graphics backend
*/

#include "engine/render/SfmlRenderer.hpp"

#include <SFML/Graphics.hpp>
#include <cmath>

#include "AssetManager.hpp"

namespace engine::render {

struct SfmlRenderer::Implementation {
	AssetManager assets;
	sf::RenderWindow window;

	explicit Implementation(const WindowConfig& config)
		: assets(config.assetRoot, config.fallbackTexture),
		  window(sf::VideoMode({config.width, config.height}), config.title)
	{
	}
};

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

/// Loads a texture before the first frame, so a missing required file fails at startup
/// instead of being drawn as the fallback texture.
void SfmlRenderer::preload(std::string_view texture)
{
	_implementation->assets.preload(texture);
}

void SfmlRenderer::beginFrame()
{
	_implementation->window.clear(sf::Color::Black);
}

/// Draws the repeated background at the offset selected by the render system.
void SfmlRenderer::drawBackground(std::string_view texture, float offset)
{
	_implementation->assets.setRepeated(texture, true);
	const sf::Texture& image = _implementation->assets.texture(texture);
	const sf::Vector2u size = _implementation->window.getSize();
	if (size.x == 0 || size.y == 0 || image.getSize().x == 0) {
		return;
	}
	const float wrapped = std::fmod(offset, static_cast<float>(image.getSize().x));
	const int left = static_cast<int>(wrapped);
	sf::Sprite sprite(image, sf::IntRect({left, 0}, {static_cast<int>(size.x) + 1, static_cast<int>(size.y)}));
	sprite.setPosition({-(wrapped - static_cast<float>(left)), 0.0F});
	_implementation->window.draw(sprite);
}

/// Translates a library independent sprite command into an SFML draw call.
void SfmlRenderer::drawSprite(const SpriteDraw& command)
{
	const Rect& region = command.region;
	sf::Sprite sprite = _implementation->assets.sprite(command.texture, sf::IntRect({region.position.x, region.position.y}, {region.size.x, region.size.y}));
	sprite.setOrigin({command.originX, command.originY});
	sprite.setScale({command.scale, command.scale});
	sprite.setRotation(sf::degrees(command.rotation));
	sprite.setPosition({command.x, command.y});
	_implementation->window.draw(sprite);
}

void SfmlRenderer::endFrame()
{
	_implementation->window.display();
}

}
