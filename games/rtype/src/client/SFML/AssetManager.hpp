/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SFML texture cache and spritesheet access
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace rtype {

class AssetManager {
private:
	static constexpr std::string_view FALLBACK_FILE = "magenta-checkerboard-200px.png";
	using TextureMap = std::unordered_map<std::filesystem::path, sf::Texture>;

	sf::Texture& loadTexture(const std::filesystem::path& name);

	std::filesystem::path _root;
	sf::Texture _fallbackTexture;
	TextureMap _textures;
	std::unordered_set<std::filesystem::path> _missing;

public:
	AssetManager();
	void preload(std::string_view name);
	const sf::Texture& texture(std::string_view name);
	void setRepeated(std::string_view name, bool repeated);
	sf::Sprite sprite(std::string_view name, const sf::IntRect& area);
};

}
