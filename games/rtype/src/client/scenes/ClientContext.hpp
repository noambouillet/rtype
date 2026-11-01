/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client services shared by every scene
*/

#pragma once

#include "client/app/ClientOptions.hpp"
#include "engine/assets/AssetManager.hpp"
#include "engine/audio/AudioBackend.hpp"
#include "engine/core/EventBus.hpp"
#include "engine/core/Logger.hpp"
#include "engine/core/scene/SceneManager.hpp"
#include "engine/input/InputBackend.hpp"
#include "engine/render/GraphicsBackend.hpp"

namespace rtype::client {

/// Everything a scene reads or drives. Every member outlives the scenes.
struct ClientContext {
	engine::core::SceneManager& scenes;
	engine::render::IGraphicsBackend& graphics;
	const engine::input::IInputBackend& input;
	engine::audio::IAudioBackend& audio;
	const engine::assets::AssetManager& assets;
	engine::core::Logger& logger;
	engine::core::EventBus& bus;
	const ClientOptions& options;
	bool screenFlash;
	engine::render::FontId font;
};

}
