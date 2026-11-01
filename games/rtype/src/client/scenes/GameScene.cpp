/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Networked game
*/

#include "GameScene.hpp"

#include <memory>

#include "client/scenes/MainMenuScene.hpp"
#include "client/scenes/PauseScene.hpp"
#include "utils/network/Protocol.hpp"

namespace rtype::client {

/// Opens the socket to the server given on the command line. Throws a NetworkError if its address cannot be resolved.
GameScene::GameScene(ClientContext& context)
	: _context(context),
	  _transport({.host = context.options.host, .port = context.options.port, .maxDatagramSize = network::MAX_DATAGRAM_SIZE}),
	  _client(_transport, context.bus),
	  _controls(),
	  _view(context.assets, context.screenFlash),
	  _events(),
	  _previous(_client.status()),
	  _buttons(0),
	  _deltaTime(0.0F)
{
}

void GameScene::onEnter()
{
	_context.logger.info("Connecting to " + _context.options.host + ':' + std::to_string(_context.options.port));
}

/// Tells the server the player leaves, whether the scene is closed from a menu or because the client stops.
void GameScene::onExit()
{
	_client.disconnect();
	_context.logger.info("Disconnected");
}

/// The server keeps running while the pause menu is open, so the ship is sent no button instead of the last ones.
void GameScene::onPause()
{
	_buttons = 0;
}

/// Reads the ship buttons sent to the server, and opens the pause menu over the game.
void GameScene::handleInput()
{
	_buttons = _controls.buttons(_context.input);
	_view.updateInterface(_context.input, _context.graphics, _events);
	if (_context.input.justPressed(ClientControls::PAUSE)) {
		_context.scenes.push(std::make_unique<PauseScene>(_context));
	}
}

/// Keeps talking to the server even under the pause menu. When the session ends, goes back to the title screen with the reason.
void GameScene::update(float deltaTime)
{
	_deltaTime = deltaTime;
	_view.update(deltaTime, _client, _buttons);
	reportNotices();
	if (_client.status() != _previous) {
		_previous = _client.status();
		if (const auto failure = reportStatus()) {
			_context.scenes.clear();
			_context.scenes.push(std::make_unique<MainMenuScene>(_context, *failure));
		}
	}
	_view.playAudio(_context.audio);
}

void GameScene::render()
{
	_view.render(_context.graphics, _deltaTime);
}

/// Writes any server notices received during the latest network update.
void GameScene::reportNotices()
{
	if (const auto departure = _client.takeDeparture()) {
		_context.logger.info("Player " + std::to_string(departure->playerId) + " left");
	}
	if (const auto error = _client.takeError()) {
		_context.logger.warn("Server protocol error " + std::to_string(static_cast<int>(*error)));
	}
}

/// Logs a change in the session state, and gives the message to show when the session ended.
std::optional<std::string> GameScene::reportStatus()
{
	switch (_client.status()) {
		case network::ClientStatus::Connected:
			_context.logger.info("Joined as player " + std::to_string(_client.playerId()));
			return std::nullopt;
		case network::ClientStatus::Rejected:
			_context.logger.error("Server rejected the join request");
			return "The server rejected the join request";
		case network::ClientStatus::TimedOut:
			_context.logger.error("Join timed out");
			return "The server did not answer";
		case network::ClientStatus::Disconnected:
			_context.logger.error("The server closed this session");
			return "The server closed the session";
		default:
			return std::nullopt;
	}
}

}
