/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client view world
*/

#pragma once

#include <cstdint>

#include "client/network/GameClient.hpp"
#include "engine/assets/AssetManager.hpp"
#include "engine/audio/AudioBackend.hpp"
#include "engine/core/EventBus.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/input/InputBackend.hpp"
#include "engine/render/GraphicsBackend.hpp"

namespace rtype::client {

class ClientWorld {
public:
	explicit ClientWorld(const engine::assets::AssetManager& assets, bool screenFlash = true);
	void update(float deltaTime, network::GameClient& client, std::uint8_t buttons);
	void updateInterface(const engine::input::IInputBackend& input, const engine::render::IGraphicsBackend& backend, engine::core::EventBus& bus);
	void render(engine::render::IGraphicsBackend& backend, float deltaTime);
	void playAudio(engine::audio::IAudioBackend& backend);

private:
	engine::core::Coordinator _coordinator;
};

}
