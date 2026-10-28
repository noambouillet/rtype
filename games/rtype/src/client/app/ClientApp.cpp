/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Graphical client application loop
*/

#include "ClientApp.hpp"

#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include "client/GameConfig.hpp"
#include "client/app/ClientControls.hpp"
#include "client/network/GameClient.hpp"
#include "client/view/ClientWorld.hpp"
#include "client/view/SpriteRegions.hpp"
#include "engine/audio/SfmlAudio.hpp"
#include "engine/core/ConfigParser.hpp"
#include "engine/core/EngineSettings.hpp"
#include "engine/core/Error.hpp"
#include "engine/core/Logger.hpp"
#include "engine/input/SfmlInput.hpp"
#include "engine/net/UdpClient.hpp"
#include "engine/render/SfmlRenderer.hpp"
#include "utils/network/Protocol.hpp"

namespace rtype {

static volatile std::sig_atomic_t running = 1;

static void stopClient([[maybe_unused]] int signal)
{
	running = 0;
}

/// Creates a separate logger for this client process.
static std::unique_ptr<engine::core::Logger> makeLogger(const std::string& logFile)
{
	if (logFile.empty()) {
		return std::make_unique<engine::core::Logger>(std::cout);
	}
	return std::make_unique<engine::core::Logger>(std::filesystem::path(logFile));
}

/// Reports a change in the client's session state.
static bool reportStatus(const network::GameClient& client, engine::core::Logger& logger)
{
	switch (client.status()) {
		case network::ClientStatus::Connected:
			logger.info("Joined as player " + std::to_string(client.playerId()));
			return true;
		case network::ClientStatus::Rejected:
			logger.error("Server rejected the join request");
			return false;
		case network::ClientStatus::TimedOut:
			logger.error("Join timed out");
			return false;
		case network::ClientStatus::Disconnected:
			logger.error("The server closed this session");
			return false;
		default:
			return true;
	}
}

/// Writes any server notices received during the latest network update.
static void reportNotices(network::GameClient& client, engine::core::Logger& logger)
{
	if (const auto departure = client.takeDeparture()) {
		logger.info("Player " + std::to_string(departure->playerId) + " left");
	}
	if (const auto error = client.takeError()) {
		logger.warn("Server protocol error " + std::to_string(static_cast<int>(*error)));
	}
}

/// Loads client settings, creating the settings file with default values if it is missing or empty.
static void loadSettings(engine::core::EngineSettings& engineSettings, engine::input::InputMap& bindings)
{
	const std::string configFile = std::string(client_config::SETTINGS_FILE);
	std::map<std::string, std::string> config = engine::core::ConfigParser::parseFile(configFile);
	engineSettings.loadFromMap(config);
	bindings.loadFromMap(config);
	if (config.empty()) {
		engineSettings.saveToMap(config);
		bindings.saveToMap(config);
		if (!engine::core::ConfigParser::saveFile(configFile, config)) {
			throw engine::core::ConfigError("Unable to create settings file: " + configFile);
		}
	}
}

/// Polls one frame and sends the current input state.
static bool updateFrame(engine::render::SfmlRenderer& window, engine::input::SfmlInput& input, engine::audio::SfmlAudio& audio,
	client::ClientControls& controls, network::GameClient& client, rtype::client::ClientWorld& view,
	engine::core::Logger& logger, network::ClientStatus& previous, std::chrono::steady_clock::time_point& lastFrame)
{
	window.handleEvent();
	input.update(window.hasFocus());
	if (input.justPressed(client::ClientControls::QUIT)) {
		window.close();
	}
	if (!window.isOpen()) {
		return true;
	}
	const auto now = std::chrono::steady_clock::now();
	const float deltaTime = std::clamp(std::chrono::duration<float>(now - lastFrame).count(), 0.0F, 0.1F);
	lastFrame = now;
	const std::uint8_t buttons = controls.buttons(input);
	view.update(deltaTime, client, buttons);
	reportNotices(client, logger);
	if (client.status() != previous) {
		previous = client.status();
		if (!reportStatus(client, logger)) {
			return false;
		}
	}
	view.render(window, deltaTime);
	view.playAudio(audio);
	return true;
}

/// Runs the graphical client until the window closes or a signal arrives.
int runClient(const ClientOptions& options)
{
	std::signal(SIGINT, stopClient);
	std::signal(SIGTERM, stopClient);
	auto logger = makeLogger(options.logFile);
	logger->setVerbose(true);
	engine::core::EngineSettings engineSettings;
	engine::input::InputMap bindings = client::ClientControls::defaultBindings();
	loadSettings(engineSettings, bindings);
	engine::net::UdpClient transport({.host = options.host, .port = options.port, .maxDatagramSize = network::MAX_DATAGRAM_SIZE});
	network::GameClient client(transport);
	engine::render::SfmlRenderer window({.width = client_config::WINDOW_WIDTH, .height = client_config::WINDOW_HEIGHT, .title = "R-Type"});
	window.preload(sprites::PLAYER_SHEET);
	window.preload(sprites::SHOT_SHEET);
	window.preload(sprites::ENEMY_SHEET);
	window.preload("background.png");
	window.preload(sprites::EXPLOSION_SHEET);
	engine::input::SfmlInput input(std::move(bindings));
	engine::audio::SfmlAudio audio("assets/sounds");
	client::ClientControls controls;
	rtype::client::ClientWorld view(engineSettings.screenFlashEnabled());
	auto previous = client.status();
	auto lastFrame = std::chrono::steady_clock::now();
	logger->info("Connecting to " + options.host + ':' + std::to_string(options.port));
	while (running != 0 && window.isOpen()) {
		if (!updateFrame(window, input, audio, controls, client, view, *logger, previous, lastFrame)) {
			return 1;
		}
	}
	client.disconnect();
	logger->info("Disconnected");
	return 0;
}

}
