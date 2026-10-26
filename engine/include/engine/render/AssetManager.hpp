/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Data-driven asset manager interface
*/

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "engine/render/GraphicsBackend.hpp"

namespace engine::render {

static constexpr std::string_view DEFAULT_FALLBACK_FILE = "magenta-checkerboard-200px.png";
using TextureId = std::uint32_t;

struct SpriteDescriptor {
	TextureId texture = 0;
	Rect rect{};
	float originX = 0.0F;
	float originY = 0.0F;
};

class AssetManager {
public:
	AssetManager() = default;

	void loadManifest(const std::string& path, IGraphicsBackend& graphics);

	const SpriteDescriptor& getSprite(const std::string& name) const;
	TextureId getTexture(const std::string& name) const;

	void update(IGraphicsBackend& graphics);

	bool hasTexture(const std::string& name) const;
	bool hasSprite(const std::string& name) const;

	TextureId fallbackTexture() const;
	const SpriteDescriptor& fallbackSprite() const;

private:
	std::unordered_map<std::string, SpriteDescriptor> _sprites;
	std::unordered_map<std::string, TextureId> _textures;
	TextureId _fallbackTexture = 0;
	SpriteDescriptor _fallbackSprite{};
	std::string _manifestPath;
	std::filesystem::file_time_type _lastWriteTime{};
};

}
