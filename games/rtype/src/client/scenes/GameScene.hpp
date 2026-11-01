/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Networked game
*/

#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "client/app/ClientControls.hpp"
#include "client/network/GameClient.hpp"
#include "client/scenes/ClientContext.hpp"
#include "client/view/ClientWorld.hpp"
#include "engine/core/EventBus.hpp"
#include "engine/core/scene/Scene.hpp"
#include "engine/net/UdpClient.hpp"

namespace rtype::client {

class GameScene : public engine::core::Scene {
public:
	explicit GameScene(ClientContext& context);

	void onEnter() override;
	void onExit() override;
	void onPause() override;
	void handleInput() override;
	void update(float deltaTime) override;
	void render() override;

private:
	void reportNotices();
	std::optional<std::string> reportStatus();

	ClientContext& _context;
	engine::net::UdpClient _transport;
	network::GameClient _client;
	ClientControls _controls;
	ClientWorld _view;
	engine::core::EventBus _events;
	network::ClientStatus _previous;
	std::uint8_t _buttons;
	float _deltaTime;
};

}
