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
#include "engine/render/GraphicsBackend.hpp"

namespace rtype::client {

class ClientWorld {
public:
	ClientWorld();
	void update(network::GameClient& client, std::uint8_t buttons, float deltaTime);
	void render(engine::render::IGraphicsBackend& backend, float deltaTime);
	void playAudio(engine::audio::IAudioBackend& backend);

private:
	engine::core::Coordinator _coordinator;
};

}
