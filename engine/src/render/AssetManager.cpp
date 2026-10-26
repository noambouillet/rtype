/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Data-driven asset manager implementation
*/

#include "engine/render/AssetManager.hpp"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "engine/core/Error.hpp"

namespace engine::render {

/// Resolves an asset file path relative to a base directory or standard asset locations.
static std::filesystem::path resolveAssetPath(const std::filesystem::path& baseDirectory,
	const std::string& rawAssetPath)
{
	const std::filesystem::path candidatePath(rawAssetPath);
	if (std::filesystem::exists(candidatePath)) {
		return candidatePath;
	}
	if (!baseDirectory.empty() && std::filesystem::exists(baseDirectory / candidatePath)) {
		return baseDirectory / candidatePath;
	}
	if (std::filesystem::exists(std::filesystem::path("assets") / candidatePath)) {
		return std::filesystem::path("assets") / candidatePath;
	}
	return baseDirectory.empty() ? candidatePath : (baseDirectory / candidatePath);
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
	const TextureId fallbackId = graphics.loadTextureFromFile(fallbackPath.string());
	if (fallbackId == 0) {
		throw engine::core::AssetError("Failed to load fallback texture: " + fallbackPath.string());
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
	const TextureId textureId = graphics.loadTextureFromFile(resolvedPath.string());
	if (textureId == 0) {
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
	if (!spriteData.contains("rect") || !spriteData["rect"].is_object()) {
		return spriteRect;
	}
	const auto& rectData = spriteData["rect"];
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

/// Parses the JSON manifest file to register all textures and sprites.
void AssetManager::loadManifest(const std::string& path, IGraphicsBackend& graphics)
{
	const std::filesystem::path manifestPath = findManifestPath(path);
	if (manifestPath.empty()) {
		throw engine::core::AssetError("Manifest file not found: " + path);
	}
	nlohmann::json manifestJson;
	if (!parseJsonFile(manifestPath, manifestJson)) {
		throw engine::core::AssetError("Failed to parse manifest: " + manifestPath.string());
	}
	_manifestPath = manifestPath.string();
	std::error_code errorCode;
	_lastWriteTime = std::filesystem::last_write_time(manifestPath, errorCode);
	const std::filesystem::path baseDirectory = manifestPath.parent_path();

	_fallbackTexture = loadFallbackTexture(manifestJson, baseDirectory, graphics);
	_fallbackSprite = SpriteDescriptor{_fallbackTexture, Rect{{0, 0}, {0, 0}}, 0.0F, 0.0F};
	_textures[std::string(DEFAULT_FALLBACK_FILE)] = _fallbackTexture;

	loadTexturesSection(manifestJson, baseDirectory, graphics, _fallbackTexture, _textures);
	loadSpritesSection(manifestJson, _fallbackTexture, _textures, _sprites);
}

/// Retrieves a sprite descriptor or logs a warning and returns the fallback sprite.
const SpriteDescriptor& AssetManager::getSprite(const std::string& name) const
{
	const auto spriteIterator = _sprites.find(name);
	if (spriteIterator != _sprites.end()) {
		return spriteIterator->second;
	}
	std::cerr << "AssetManager warning: Requested sprite '" << name << "' is missing, returning fallback ID\n";
	return _fallbackSprite;
}

/// Retrieves a texture ID or logs a warning and returns the fallback texture ID.
TextureId AssetManager::getTexture(const std::string& name) const
{
	const auto textureIterator = _textures.find(name);
	if (textureIterator != _textures.end()) {
		return textureIterator->second;
	}
	std::cerr << "AssetManager warning: Requested texture '" << name << "' is missing, returning fallback ID\n";
	return _fallbackTexture;
}

/// Hot-reloads the manifest file if its modification timestamp has changed.
void AssetManager::update(IGraphicsBackend& graphics)
{
	if (_manifestPath.empty()) {
		return;
	}
	std::error_code errorCode;
	const auto currentTime = std::filesystem::last_write_time(_manifestPath, errorCode);
	if (!errorCode && currentTime != _lastWriteTime) {
		_lastWriteTime = currentTime;
		std::cout << "AssetManager: Manifest modification detected, hot-reloading...\n";
		try {
			loadManifest(_manifestPath, graphics);
		} catch (const engine::core::AssetError& error) {
			std::cerr << "AssetManager warning: Failed to hot-reload manifest: " << error.what() << "\n";
		}
	}
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

}
