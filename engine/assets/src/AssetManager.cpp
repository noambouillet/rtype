/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Data-driven asset manager implementation
*/

#include "engine/assets/AssetManager.hpp"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "engine/assets/AssetError.hpp"

namespace engine::assets {

/// Resolves an asset file path relative to a base directory or standard asset locations.
/// Returns a relative path if found, or an empty path if it does not exist.
static std::filesystem::path resolveAssetPath(const std::filesystem::path& baseDirectory,
	const std::string& rawAssetPath)
{
	if (rawAssetPath.empty()) {
		return {};
	}
	const std::filesystem::path candidatePath(rawAssetPath);
	if (candidatePath.is_absolute() && std::filesystem::exists(candidatePath)) {
		return candidatePath;
	}
	if (!baseDirectory.empty() && std::filesystem::exists(baseDirectory / candidatePath)) {
		return baseDirectory / candidatePath;
	}
	if (std::filesystem::exists(std::filesystem::path("assets") / candidatePath)) {
		return std::filesystem::path("assets") / candidatePath;
	}
	if (std::filesystem::exists(candidatePath)) {
		return candidatePath;
	}
	return {};
}

/// Finds an existing manifest path from user input or fallback locations.
static std::filesystem::path findManifestPath(const std::string& manifestPath)
{
	if (std::filesystem::exists(manifestPath)) {
		return manifestPath;
	}
	const std::filesystem::path standardPath = std::filesystem::path("assets") / manifestPath;
	if (std::filesystem::exists(standardPath)) {
		return standardPath;
	}
	return {};
}

/// Reads and parses a JSON file from disk into a json object.
static bool parseJsonFile(const std::filesystem::path& filePath, nlohmann::json& parsedJson)
{
	std::ifstream manifestFile(filePath);
	if (!manifestFile.is_open()) {
		std::cerr << "AssetManager warning: Unable to open manifest: " << filePath << "\n";
		return false;
	}
	try {
		manifestFile >> parsedJson;
		return true;
	} catch (const nlohmann::json::exception& exception) {
		std::cerr << "AssetManager warning: JSON parse error in manifest: " << exception.what() << "\n";
		return false;
	}
}

/// Resolves and loads the fallback texture.
static TextureId loadFallbackTexture(const nlohmann::json& manifestJson,
	const std::filesystem::path& baseDirectory, IGraphicsBackend& graphics)
{
	std::string fallbackFilename = std::string(DEFAULT_FALLBACK_FILE);
	if (manifestJson.contains("fallback") && manifestJson["fallback"].is_string()) {
		fallbackFilename = manifestJson["fallback"].get<std::string>();
	}
	const std::filesystem::path fallbackPath = resolveAssetPath(baseDirectory, fallbackFilename);
	if (fallbackPath.empty() || !std::filesystem::exists(fallbackPath)) {
		throw AssetError("Failed to find fallback texture: " + fallbackFilename);
	}
	const TextureId fallbackId = graphics.loadTextureFromFile(fallbackPath.string());
	if (fallbackId == INVALID_TEXTURE_ID) {
		throw AssetError("Failed to load fallback texture: " + fallbackPath.string());
	}
	return fallbackId;
}

/// Loads a single texture entry into the textures map.
static void loadTextureEntry(const std::string& textureKey, const nlohmann::json& textureValue,
	const std::filesystem::path& baseDirectory, IGraphicsBackend& graphics, TextureId fallbackId,
	std::unordered_map<std::string, TextureId>& loadedTextures)
{
	std::string relativePath;
	if (textureValue.is_string()) {
		relativePath = textureValue.get<std::string>();
	} else if (textureValue.is_object() && textureValue.contains("path") && textureValue["path"].is_string()) {
		relativePath = textureValue["path"].get<std::string>();
	} else {
		return;
	}
	const std::filesystem::path resolvedPath = resolveAssetPath(baseDirectory, relativePath);
	if (resolvedPath.empty() || !std::filesystem::exists(resolvedPath)) {
		std::cerr << "AssetManager warning: Missing texture asset '" << textureKey << "', using fallback ID\n";
		loadedTextures[textureKey] = fallbackId;
		return;
	}
	const TextureId textureId = graphics.loadTextureFromFile(resolvedPath.string());
	if (textureId == INVALID_TEXTURE_ID) {
		std::cerr << "AssetManager warning: Missing texture asset '" << textureKey << "', using fallback ID\n";
		loadedTextures[textureKey] = fallbackId;
		return;
	}
	loadedTextures[textureKey] = textureId;
	loadedTextures[resolvedPath.filename().string()] = textureId;
	loadedTextures[relativePath] = textureId;
}

/// Iterates and registers all textures from the JSON object.
static void loadTexturesSection(const nlohmann::json& manifestJson, const std::filesystem::path& baseDirectory,
	IGraphicsBackend& graphics, TextureId fallbackId, std::unordered_map<std::string, TextureId>& loadedTextures)
{
	if (!manifestJson.contains("textures") || !manifestJson["textures"].is_object()) {
		return;
	}
	for (const auto& [textureKey, textureValue] : manifestJson["textures"].items()) {
		loadTextureEntry(textureKey, textureValue, baseDirectory, graphics, fallbackId, loadedTextures);
	}
}

/// Extracts the rectangle bounds from a sprite JSON description.
static Rect extractSpriteRect(const nlohmann::json& spriteData)
{
	Rect spriteRect{{0, 0}, {0, 0}};
	const auto& rectData = (spriteData.contains("rect") && spriteData["rect"].is_object())
							   ? spriteData["rect"]
							   : spriteData;
	spriteRect.position.x = rectData.value("x", 0);
	spriteRect.position.y = rectData.value("y", 0);
	spriteRect.size.x = rectData.contains("width") ? rectData["width"].get<int>() : rectData.value("w", 0);
	spriteRect.size.y = rectData.contains("height") ? rectData["height"].get<int>() : rectData.value("h", 0);
	return spriteRect;
}

/// Extracts the pivot origin offsets from a sprite JSON description.
static void extractSpriteOrigin(const nlohmann::json& spriteData, float& originX, float& originY)
{
	if (spriteData.contains("origin") && spriteData["origin"].is_object()) {
		const auto& originData = spriteData["origin"];
		originX = originData.value("x", 0.0F);
		originY = originData.value("y", 0.0F);
		return;
	}
	originX = spriteData.value("originX", 0.0F);
	originY = spriteData.value("originY", 0.0F);
}

/// Resolves a texture identifier referenced by a sprite.
static TextureId resolveSpriteTexture(const std::string& spriteName, const nlohmann::json& spriteData,
	TextureId fallbackId, const std::unordered_map<std::string, TextureId>& loadedTextures)
{
	const std::string textureReference = spriteData.value("texture", "");
	if (textureReference.empty()) {
		return fallbackId;
	}
	const auto textureIterator = loadedTextures.find(textureReference);
	if (textureIterator != loadedTextures.end()) {
		return textureIterator->second;
	}
	std::cerr << "AssetManager warning: Sprite '" << spriteName << "' unknown texture '" << textureReference << "'\n";
	return fallbackId;
}

/// Parses a single sprite descriptor and inserts it into the map.
static void loadSpriteEntry(const std::string& spriteName, const nlohmann::json& spriteData,
	TextureId fallbackId, const std::unordered_map<std::string, TextureId>& loadedTextures,
	std::unordered_map<std::string, SpriteDescriptor>& loadedSprites)
{
	if (!spriteData.is_object()) {
		return;
	}
	const TextureId textureId = resolveSpriteTexture(spriteName, spriteData, fallbackId, loadedTextures);
	const Rect spriteRect = extractSpriteRect(spriteData);
	float originX = 0.0F;
	float originY = 0.0F;
	extractSpriteOrigin(spriteData, originX, originY);
	loadedSprites[spriteName] = SpriteDescriptor{textureId, spriteRect, originX, originY};
}

/// Iterates and registers all sprites from the JSON object.
static void loadSpritesSection(const nlohmann::json& manifestJson, TextureId fallbackId,
	const std::unordered_map<std::string, TextureId>& loadedTextures,
	std::unordered_map<std::string, SpriteDescriptor>& loadedSprites)
{
	if (!manifestJson.contains("sprites") || !manifestJson["sprites"].is_object()) {
		return;
	}
	for (const auto& [spriteName, spriteData] : manifestJson["sprites"].items()) {
		loadSpriteEntry(spriteName, spriteData, fallbackId, loadedTextures, loadedSprites);
	}
}

/// Iterates and registers all animation descriptors from the JSON object.
static void loadAnimationsSection(const nlohmann::json& manifestJson, TextureId fallbackId,
	const std::unordered_map<std::string, TextureId>& loadedTextures,
	std::unordered_map<std::string, AnimationDescriptor>& loadedAnimations)
{
	if (!manifestJson.contains("animations") || !manifestJson["animations"].is_object()) {
		return;
	}
	for (const auto& [animName, animData] : manifestJson["animations"].items()) {
		if (!animData.is_object()) {
			continue;
		}
		AnimationDescriptor anim{};
		anim.name = animName;
		anim.texture = resolveSpriteTexture(animName, animData, fallbackId, loadedTextures);
		anim.frameSeconds = animData.value("frameSeconds", 0.1F);
		anim.loop = animData.value("loop", false);
		if (animData.contains("frames") && animData["frames"].is_array()) {
			for (const auto& frameData : animData["frames"]) {
				if (frameData.is_object()) {
					anim.frames.push_back(extractSpriteRect(frameData));
				}
			}
		}
		loadedAnimations[animName] = std::move(anim);
	}
}

/// Iterates and registers all sound filepaths from the JSON object.
static void loadSoundsSection(const nlohmann::json& manifestJson, const std::filesystem::path& baseDirectory,
	std::unordered_map<std::string, std::string>& soundPaths)
{
	if (!manifestJson.contains("sounds") || !manifestJson["sounds"].is_object()) {
		return;
	}
	for (const auto& [soundKey, soundValue] : manifestJson["sounds"].items()) {
		if (soundValue.is_string()) {
			const std::filesystem::path resolved = resolveAssetPath(baseDirectory, soundValue.get<std::string>());
			if (!resolved.empty()) {
				soundPaths[soundKey] = resolved.string();
			} else {
				soundPaths[soundKey] = (baseDirectory / soundValue.get<std::string>()).string();
			}
		}
	}
}

/// Iterates and registers all font filepaths from the JSON object.
static void loadFontsSection(const nlohmann::json& manifestJson, const std::filesystem::path& baseDirectory,
	std::unordered_map<std::string, std::string>& fontPaths)
{
	if (!manifestJson.contains("fonts") || !manifestJson["fonts"].is_object()) {
		return;
	}
	for (const auto& [fontKey, fontValue] : manifestJson["fonts"].items()) {
		if (fontValue.is_string()) {
			const std::filesystem::path resolved = resolveAssetPath(baseDirectory, fontValue.get<std::string>());
			if (!resolved.empty()) {
				fontPaths[fontKey] = resolved.string();
			} else {
				fontPaths[fontKey] = (baseDirectory / fontValue.get<std::string>()).string();
			}
		}
	}
}

/// Iterates and registers preload groups from the JSON object.
static void loadGroupsSection(const nlohmann::json& manifestJson,
	std::unordered_map<std::string, ManifestGroup>& groups)
{
	if (!manifestJson.contains("groups") || !manifestJson["groups"].is_object()) {
		return;
	}
	for (const auto& [groupKey, groupVal] : manifestJson["groups"].items()) {
		if (!groupVal.is_object()) {
			continue;
		}
		ManifestGroup group{};
		group.required = groupVal.value("required", false);
		if (groupVal.contains("textures") && groupVal["textures"].is_array()) {
			for (const auto& item : groupVal["textures"]) {
				if (item.is_string()) {
					group.textures.push_back(item.get<std::string>());
				}
			}
		}
		if (groupVal.contains("sounds") && groupVal["sounds"].is_array()) {
			for (const auto& item : groupVal["sounds"]) {
				if (item.is_string()) {
					group.sounds.push_back(item.get<std::string>());
				}
			}
		}
		groups[groupKey] = std::move(group);
	}
}

/// Helper that registers a single tag for a given key, resolving texture IDs and aliases.
static void registerSingleTag(const std::string& key, const std::string& tagName,
	const nlohmann::json& manifestJson,
	std::unordered_map<std::string, std::unordered_set<std::string>>& assetTags,
	const std::unordered_map<std::string, TextureId>& loadedTextures,
	std::unordered_map<TextureId, std::unordered_set<std::string>>& textureTags)
{
	assetTags[key].insert(tagName);

	const auto textureIterator = loadedTextures.find(key);
	if (textureIterator != loadedTextures.end()) {
		textureTags[textureIterator->second].insert(tagName);
	}

	if (manifestJson.contains("textures") && manifestJson["textures"].is_object() &&
		manifestJson["textures"].contains(key) && manifestJson["textures"][key].is_string()) {
		const std::string fileName = manifestJson["textures"][key].get<std::string>();
		assetTags[fileName].insert(tagName);

		const auto fileIterator = loadedTextures.find(fileName);
		if (fileIterator != loadedTextures.end()) {
			textureTags[fileIterator->second].insert(tagName);
		}
	}
}

/// Iterates and registers tags from the JSON object.
static void loadTagsSection(const nlohmann::json& manifestJson,
	std::unordered_map<std::string, std::unordered_set<std::string>>& assetTags,
	const std::unordered_map<std::string, TextureId>& loadedTextures,
	std::unordered_map<TextureId, std::unordered_set<std::string>>& textureTags)
{
	if (!manifestJson.contains("tags") || !manifestJson["tags"].is_object()) {
		return;
	}

	for (const auto& [tagName, tagItems] : manifestJson["tags"].items()) {
		if (!tagItems.is_array()) {
			continue;
		}
		for (const auto& item : tagItems) {
			if (item.is_string()) {
				registerSingleTag(item.get<std::string>(), tagName, manifestJson, assetTags, loadedTextures, textureTags);
			}
		}
	}
}

/// Parses the JSON manifest file to register all textures and sprites with strong exception safety.
void AssetManager::loadManifest(const std::string& path, IGraphicsBackend& graphics)
{
	const std::filesystem::path manifestPath = findManifestPath(path);
	if (manifestPath.empty()) {
		throw AssetError("Manifest file not found: " + path);
	}
	nlohmann::json manifestJson;
	if (!parseJsonFile(manifestPath, manifestJson)) {
		throw AssetError("Failed to parse manifest: " + manifestPath.string());
	}
	const std::filesystem::path baseDirectory = manifestPath.parent_path();

	const TextureId fallbackTexture = loadFallbackTexture(manifestJson, baseDirectory, graphics);
	SpriteDescriptor fallbackSprite{fallbackTexture, Rect{{0, 0}, {200, 200}}, 100.0F, 100.0F};
	AnimationDescriptor fallbackAnimation{"fallback", fallbackTexture, {}, 0.0F, false};

	std::unordered_map<std::string, SpriteDescriptor> sprites;
	std::unordered_map<std::string, TextureId> textures;
	std::unordered_map<std::string, AnimationDescriptor> animations;
	std::unordered_map<std::string, std::string> soundPaths;
	std::unordered_map<std::string, std::string> fontPaths;
	std::unordered_map<std::string, ManifestGroup> groups;
	std::unordered_map<std::string, std::unordered_set<std::string>> assetTags;
	std::unordered_map<TextureId, std::unordered_set<std::string>> textureTags;

	textures[std::string(DEFAULT_FALLBACK_FILE)] = fallbackTexture;

	loadTexturesSection(manifestJson, baseDirectory, graphics, fallbackTexture, textures);
	loadSpritesSection(manifestJson, fallbackTexture, textures, sprites);
	loadAnimationsSection(manifestJson, fallbackTexture, textures, animations);
	loadSoundsSection(manifestJson, baseDirectory, soundPaths);
	loadFontsSection(manifestJson, baseDirectory, fontPaths);
	loadGroupsSection(manifestJson, groups);
	loadTagsSection(manifestJson, assetTags, textures, textureTags);

	_manifestPath = manifestPath.string();
	_baseDirectory = baseDirectory;
	_fallbackTexture = fallbackTexture;
	_fallbackSprite = std::move(fallbackSprite);
	_fallbackAnimation = std::move(fallbackAnimation);
	_sprites = std::move(sprites);
	_textures = std::move(textures);
	_animations = std::move(animations);
	_soundPaths = std::move(soundPaths);
	_fontPaths = std::move(fontPaths);
	_groups = std::move(groups);
	_assetTags = std::move(assetTags);
	_textureTags = std::move(textureTags);
}

/// Helper to preload all textures of a specific manifest group.
static void preloadGroupTextures(const ManifestGroup& group, const std::filesystem::path& baseDirectory,
	TextureId fallbackTexture, IGraphicsBackend& graphics, std::unordered_map<std::string, TextureId>& textures)
{
	for (const auto& textureKey : group.textures) {
		TextureId id = INVALID_TEXTURE_ID;
		const auto textureIt = textures.find(textureKey);
		if (textureIt != textures.end()) {
			id = textureIt->second;
		} else {
			const std::filesystem::path resolved = resolveAssetPath(baseDirectory, textureKey);
			if (!resolved.empty() && std::filesystem::exists(resolved)) {
				id = graphics.loadTextureFromFile(resolved.string());
				if (id != INVALID_TEXTURE_ID) {
					textures[textureKey] = id;
				}
			}
		}

		if (id == INVALID_TEXTURE_ID || (id == fallbackTexture && textureKey != DEFAULT_FALLBACK_FILE && textureKey != CHECKERBOARD_KEY)) {
			if (group.required) {
				throw AssetError("Failed to load required texture from group: " + textureKey);
			}
			std::cerr << "AssetManager warning: Preload failed for texture '" << textureKey << "'\n";
			textures[textureKey] = fallbackTexture;
		}
	}
}

/// Helper to preload all sounds of a specific manifest group.
static void preloadGroupSounds(const ManifestGroup& group, const std::filesystem::path& baseDirectory,
	std::unordered_map<std::string, std::string>& soundPaths)
{
	for (const auto& soundKey : group.sounds) {
		std::filesystem::path resolved;
		const auto soundIterator = soundPaths.find(soundKey);
		if (soundIterator != soundPaths.end()) {
			resolved = soundIterator->second;
		} else {
			resolved = resolveAssetPath(baseDirectory, soundKey);
		}

		if (resolved.empty() || !std::filesystem::exists(resolved)) {
			if (group.required) {
				throw AssetError("Failed to load required sound from group: " + soundKey);
			}
			std::cerr << "AssetManager warning: Preload failed for sound '" << soundKey << "'\n";
		} else if (soundIterator == soundPaths.end()) {
			soundPaths[soundKey] = resolved.string();
		}
	}
}

/// Preloads assets belonging to a specified group.
void AssetManager::preload(const std::string& group, IGraphicsBackend& graphics)
{
	const auto groupIterator = _groups.find(group);
	if (groupIterator == _groups.end()) {
		return;
	}

	preloadGroupTextures(groupIterator->second, _baseDirectory, _fallbackTexture, graphics, _textures);
	preloadGroupSounds(groupIterator->second, _baseDirectory, _soundPaths);
}

/// Retrieves a sprite descriptor or logs a warning and returns the fallback sprite.
const SpriteDescriptor& AssetManager::getSprite(const std::string& name) const
{
	const auto spriteIterator = _sprites.find(name);
	if (spriteIterator != _sprites.end()) {
		return spriteIterator->second;
	}
	std::cerr << "AssetManager warning: Requested sprite '" << name << "' is missing, returning fallback\n";
	return _fallbackSprite;
}

/// Retrieves a texture identifier by its key name.
TextureId AssetManager::getTexture(const std::string& name) const
{
	const auto textureIterator = _textures.find(name);
	if (textureIterator != _textures.end()) {
		return textureIterator->second;
	}
	std::cerr << "AssetManager warning: Requested texture '" << name << "' is missing, returning fallback\n";
	return _fallbackTexture;
}

/// Retrieves an animation descriptor or logs a warning and returns the fallback animation.
const AnimationDescriptor& AssetManager::getAnimation(const std::string& name) const
{
	const auto animIterator = _animations.find(name);
	if (animIterator != _animations.end()) {
		return animIterator->second;
	}
	std::cerr << "AssetManager warning: Requested animation '" << name << "' is missing, returning fallback\n";
	return _fallbackAnimation;
}

/// Retrieves a resolved sound file path by name.
const std::string& AssetManager::getSoundPath(const std::string& name) const
{
	const auto soundIterator = _soundPaths.find(name);
	if (soundIterator != _soundPaths.end()) {
		return soundIterator->second;
	}
	static const std::string EMPTY_PATH;
	return EMPTY_PATH;
}

/// Retrieves a resolved font file path by name.
const std::string& AssetManager::getFontPath(const std::string& name) const
{
	const auto fontIterator = _fontPaths.find(name);
	if (fontIterator != _fontPaths.end()) {
		return fontIterator->second;
	}
	static const std::string EMPTY_PATH;
	return EMPTY_PATH;
}

/// Checks whether a texture exists by name.
bool AssetManager::hasTexture(const std::string& name) const
{
	return _textures.find(name) != _textures.end();
}

/// Checks whether a sprite exists by name.
bool AssetManager::hasSprite(const std::string& name) const
{
	return _sprites.find(name) != _sprites.end();
}

/// Checks whether an animation exists by name.
bool AssetManager::hasAnimation(const std::string& name) const
{
	return _animations.find(name) != _animations.end();
}

/// Checks whether a sound exists by name.
bool AssetManager::hasSound(const std::string& name) const
{
	return _soundPaths.find(name) != _soundPaths.end();
}

/// Checks whether a font exists by name.
bool AssetManager::hasFont(const std::string& name) const
{
	return _fontPaths.find(name) != _fontPaths.end();
}

/// Checks whether an asset key or its associated texture has the specified tag.
bool AssetManager::hasTag(const std::string& key, const std::string& tag) const
{
	const auto tagIt = _assetTags.find(key);
	if (tagIt != _assetTags.end() && tagIt->second.contains(tag)) {
		return true;
	}
	const auto spriteIt = _sprites.find(key);
	if (spriteIt != _sprites.end()) {
		return hasTag(spriteIt->second.texture, tag);
	}
	return false;
}

/// Checks whether a texture identifier has the specified tag.
bool AssetManager::hasTag(TextureId id, const std::string& tag) const
{
	const auto tagIt = _textureTags.find(id);
	if (tagIt != _textureTags.end() && tagIt->second.contains(tag)) {
		return true;
	}
	return false;
}

/// Returns the identifier of the fallback texture.
TextureId AssetManager::fallbackTexture() const
{
	return _fallbackTexture;
}

/// Returns the descriptor of the fallback sprite.
const SpriteDescriptor& AssetManager::fallbackSprite() const
{
	return _fallbackSprite;
}

/// Returns the descriptor of the fallback animation.
const AnimationDescriptor& AssetManager::fallbackAnimation() const
{
	return _fallbackAnimation;
}

}
