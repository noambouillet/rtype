/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SFML texture cache and spritesheet access
*/

#include "AssetManager.hpp"

#include <cstdint>
#include <iostream>
#include <string>

#include "utils/RTypeError.hpp"

namespace rtype {

AssetManager::AssetManager() : _root("assets")
{
	const std::filesystem::path fallbackPath = _root / std::filesystem::path(FALLBACK_FILE);
	if (!_fallbackTexture.loadFromFile(fallbackPath)) {
		throw GraphicsError("Cannot load fallback asset: " + fallbackPath.string());
	}
}

/// Loads a required sheet before rendering starts.
void AssetManager::preload(std::string_view name)
{
	const std::filesystem::path key = std::filesystem::path(name).lexically_normal();
	if (key == std::filesystem::path(FALLBACK_FILE)) {
		return;
	}
	loadTexture(key);
	_missing.erase(key);
}

/// Sets whether a required texture repeats outside its image bounds.
void AssetManager::setRepeated(std::string_view name, bool repeated)
{
	const std::filesystem::path key = std::filesystem::path(name).lexically_normal();
	loadTexture(key).setRepeated(repeated);
	_missing.erase(key);
}

/// Loads "name" into the map or throws if the file cannot be read.
sf::Texture& AssetManager::loadTexture(const std::filesystem::path& name)
{
	const std::filesystem::path key = name.lexically_normal();
	const std::filesystem::path assetPath = _root / key;
	std::pair<TextureMap::iterator, bool> result = _textures.try_emplace(key);
	if (result.second && !result.first->second.loadFromFile(assetPath)) {
		_textures.erase(result.first);
		throw GraphicsError("Cannot load asset: " + assetPath.string());
	}
	return result.first->second;
}

/// Returns a mapped texture, or the checkerboard if an optional sheet is missing.
const sf::Texture& AssetManager::texture(std::string_view name)
{
	const std::filesystem::path key = std::filesystem::path(name).lexically_normal();
	if (key == std::filesystem::path(FALLBACK_FILE) || _missing.contains(key)) {
		return _fallbackTexture;
	}
	try {
		return loadTexture(key);
	} catch (const GraphicsError& error) {
		_missing.insert(key);
		std::cerr << error.what() << "; using fallback texture\n";
		return _fallbackTexture;
	}
}

/// Creates a sprite from one region of a mapped spritesheet without copying pixels.
sf::Sprite AssetManager::sprite(std::string_view name, const sf::IntRect& area)
{
	if (area.position.x < 0 || area.position.y < 0 || area.size.x <= 0 || area.size.y <= 0) {
		throw GraphicsError("Invalid sprite region in asset: " + std::string(name));
	}
	const sf::Texture& sheet = texture(name);
	const sf::Vector2u size = sheet.getSize();
	if (&sheet == &_fallbackTexture) {
		sf::Sprite placeholder(_fallbackTexture);
		placeholder.setScale({static_cast<float>(area.size.x) / static_cast<float>(size.x), static_cast<float>(area.size.y) / static_cast<float>(size.y)});
		return placeholder;
	}
	const std::int64_t right = static_cast<std::int64_t>(area.position.x) + area.size.x;
	const std::int64_t bottom = static_cast<std::int64_t>(area.position.y) + area.size.y;
	const std::int64_t sheetWidth = size.x;
	const std::int64_t sheetHeight = size.y;
	if (right > sheetWidth || bottom > sheetHeight) {
		throw GraphicsError("Sprite region exceeds asset: " + std::string(name));
	}
	return {sheet, area};
}

}
