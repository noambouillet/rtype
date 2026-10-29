/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML graphics backend
*/

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "engine/render/GraphicsBackend.hpp"

namespace engine::render {

struct WindowConfig {
	std::uint32_t width = 1280;
	std::uint32_t height = 720;
	std::string title = "Engine";
	std::uint32_t frameRateLimit = 60;
};

class SfmlRenderer : public IGraphicsBackend {
public:
	explicit SfmlRenderer(const WindowConfig& config);
	~SfmlRenderer() override;
	SfmlRenderer(const SfmlRenderer&) = delete;
	SfmlRenderer& operator=(const SfmlRenderer&) = delete;

	bool isOpen() const;
	bool hasFocus() const;
	void close();
	void handleEvent();

	TextureId loadTextureFromFile(const std::string& filepath) override;
	void beginFrame() override;
	void drawBackground(TextureId texture, float offset) override;
	void drawSprite(const SpriteDraw& command) override;
	void endFrame() override;

private:
	// Holds the SFML objects, so code including this header does not depend on SFML.
	struct Implementation;
	std::unique_ptr<Implementation> _implementation;
};

}
