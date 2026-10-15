/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client view world
*/

#pragma once

#include <cstdint>

#include "Coordinator.hpp"
#include "client/network/GameClient.hpp"
#include "game_engine/Audio/AudioBackend.hpp"
#include "game_engine/Graphics/GraphicsBackend.hpp"

namespace rtype::client {

class ClientWorld {
public:
	ClientWorld();
	void update(network::GameClient& client, std::uint8_t buttons, float deltaTime);
	void render(render::IGraphicsBackend& backend, float deltaTime);
	void playAudio(audio::IAudioBackend& backend);

private:
	ecs::Coordinator _coordinator;
};

}
