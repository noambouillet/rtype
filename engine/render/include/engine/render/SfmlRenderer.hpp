/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML graphics backend
*/

#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include "engine/render/GraphicsBackend.hpp"

namespace engine::render {

struct WindowConfig {
	std::uint32_t width = 1280;
	std::uint32_t height = 720;
	std::string title = "Engine";
	std::uint32_t frameRateLimit = 60;
	std::filesystem::path assetRoot = "assets";
	// Drawn in place of textures that cannot be loaded. It must exist under assetRoot.
	std::filesystem::path fallbackTexture = "magenta-checkerboard-200px.png";
	std::filesystem::path fontFile;  // Leave empty to disable text.
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
	void preload(std::string_view texture);

	void beginFrame() override;
	void drawBackground(std::string_view texture, float offset) override;
	void drawSprite(const SpriteDraw& command) override;
	void drawRect(const RectDraw& rect) override;
	void drawText(const TextDraw& text) override;
	void endFrame() override;

private:
	// Holds the SFML objects, so code including this header does not depend on SFML.
	struct Implementation;
	std::unique_ptr<Implementation> _implementation;
};

}
