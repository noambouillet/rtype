/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client view world
*/

#pragma once

#include <cstdint>

#include "client/network/GameClient.hpp"
#include "engine/audio/AudioBackend.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/render/AssetManager.hpp"
#include "engine/render/GraphicsBackend.hpp"

namespace rtype::client {

class ClientWorld {
public:
	explicit ClientWorld(const engine::render::AssetManager& assets, bool screenFlash = true);
	void update(float deltaTime, network::GameClient& client, std::uint8_t buttons);
	void render(engine::render::IGraphicsBackend& backend, float deltaTime);
	void playAudio(engine::audio::IAudioBackend& backend);

private:
	engine::core::Coordinator _coordinator;
};

}
