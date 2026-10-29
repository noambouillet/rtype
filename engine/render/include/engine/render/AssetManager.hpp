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
#include <unordered_set>
#include <vector>

#include "engine/render/GraphicsBackend.hpp"

namespace engine::render {

static constexpr std::string_view DEFAULT_FALLBACK_FILE = "magenta-checkerboard-200px.png";
using TextureId = std::uint32_t;

struct SpriteDescriptor {
	TextureId texture = INVALID_TEXTURE_ID;
	Rect rect{};
	float originX = 0.0F;
	float originY = 0.0F;
};

struct AnimationDescriptor {
	std::string name;
	TextureId texture = INVALID_TEXTURE_ID;
	std::vector<Rect> frames;
	float frameSeconds = 0.0F;
	bool loop = false;
};

struct ManifestGroup {
	std::vector<std::string> textures;
	std::vector<std::string> sounds;
	bool required = false;
};

class AssetManager {
public:
	AssetManager() = default;

	void loadManifest(const std::string& path, IGraphicsBackend& graphics);

	void preload(const std::string& group, IGraphicsBackend& graphics);

	const SpriteDescriptor& getSprite(const std::string& name) const;
	TextureId getTexture(const std::string& name) const;
	const AnimationDescriptor& getAnimation(const std::string& name) const;
	const std::string& getSoundPath(const std::string& name) const;

	void update(IGraphicsBackend& graphics);

	bool hasTexture(const std::string& name) const;
	bool hasSprite(const std::string& name) const;
	bool hasAnimation(const std::string& name) const;
	bool hasSound(const std::string& name) const;
	[[nodiscard]] bool hasTag(const std::string& key, const std::string& tag) const;
	[[nodiscard]] bool hasTag(TextureId id, const std::string& tag) const;

	TextureId fallbackTexture() const;
	const SpriteDescriptor& fallbackSprite() const;
	const AnimationDescriptor& fallbackAnimation() const;

private:
	std::unordered_map<std::string, SpriteDescriptor> _sprites;
	std::unordered_map<std::string, TextureId> _textures;
	std::unordered_map<std::string, AnimationDescriptor> _animations;
	std::unordered_map<std::string, std::string> _soundPaths;
	std::unordered_map<std::string, ManifestGroup> _groups;
	std::unordered_map<std::string, std::unordered_set<std::string>> _assetTags;
	std::unordered_map<TextureId, std::unordered_set<std::string>> _textureTags;
	std::filesystem::path _baseDirectory;

	TextureId _fallbackTexture = INVALID_TEXTURE_ID;
	SpriteDescriptor _fallbackSprite{};
	AnimationDescriptor _fallbackAnimation{};
	std::string _manifestPath;
	std::filesystem::file_time_type _lastWriteTime{};
};

}
