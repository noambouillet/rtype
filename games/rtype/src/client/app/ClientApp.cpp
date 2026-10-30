/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Graphical client application loop
*/

#include "ClientApp.hpp"

#include <algorithm>
#include <array>
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
#include "engine/assets/AssetManager.hpp"
#include "engine/audio/SfmlAudio.hpp"
#include "engine/core/ConfigParser.hpp"
#include "engine/core/EngineSettings.hpp"
#include "engine/core/Error.hpp"
#include "engine/core/EventBus.hpp"
#include "engine/core/Logger.hpp"
#include "engine/core/metrics/Metrics.hpp"
#include "engine/input/SfmlInput.hpp"
#include "engine/net/UdpClient.hpp"
#include "engine/render/SfmlRenderer.hpp"
#include "engine/tools/MetricsOverlay.hpp"
#include "utils/network/Protocol.hpp"

namespace rtype {

static volatile std::sig_atomic_t running = 1;

using Clock = std::chrono::steady_clock;

/// Everything one client frame reads or updates.
struct ClientLoop {
	engine::render::SfmlRenderer& window;
	engine::input::SfmlInput& input;
	engine::audio::SfmlAudio& audio;
	client::ClientControls& controls;
	network::GameClient& client;
	client::ClientWorld& view;
	engine::core::Logger& logger;
	engine::core::EventBus& bus;
	engine::tools::MetricsOverlay& overlay;
	network::ClientStatus previous;
	Clock::time_point lastFrame;
};

static void stopClient([[maybe_unused]] int signal)
{
	running = 0;
}

/// Reports a change in the client's session state.
static bool reportStatus(const network::GameClient& client, engine::core::Logger& logger)
{
	switch (client.status()) {
		case network::ClientStatus::Connected:
			logger.info("Joined as player " + std::to_string(client.playerId()) + " (Session ID: " + std::to_string(client.sessionId()) + ")");
			return true;
		case network::ClientStatus::Rejected:
			logger.error("Server rejected the join request");
			return false;
		case network::ClientStatus::TimedOut:
			logger.warn("Join connection timed out");
			return false;
		case network::ClientStatus::Disconnected:
			logger.warn("The server closed this session (disconnected)");
			return false;
		default:
			return true;
	}
}

/// Writes any server notices received during the latest network update.
static void reportNotices(network::GameClient& client, engine::core::Logger& logger)
{
	if (const auto departure = client.takeDeparture()) {
		logger.info("Player " + std::to_string(departure->playerId) + " left the game");
	}
	if (const auto error = client.takeError()) {
		logger.warn("Server protocol error received: code " + std::to_string(static_cast<int>(*error)));
	}
}

/// Loads client settings, creating the settings file with default values if it is missing or empty.
static void loadSettings(engine::core::EngineSettings& engineSettings, engine::input::InputMap& bindings, engine::core::Logger& logger)
{
	const std::string configFile = std::string(client_config::SETTINGS_FILE);
	logger.debug("Loading configuration from " + configFile);
	std::map<std::string, std::string> config = engine::core::ConfigParser::parseFile(configFile);
	engineSettings.loadFromMap(config);
	bindings.loadFromMap(config);
	if (config.empty()) {
		logger.info("Configuration file empty or not found; generating default settings at " + configFile);
		engineSettings.saveToMap(config);
		bindings.saveToMap(config);
		if (!engine::core::ConfigParser::saveFile(configFile, config)) {
			logger.error("Unable to save settings file: " + configFile);
			throw engine::core::ConfigError("Unable to create settings file: " + configFile);
		}
	}
	logger.debug("Configuration loaded successfully");
}

/// Polls one frame, sends the current input state, draws the frame and reports its timing to the metrics.
static bool updateFrame(ClientLoop& loop)
{
	const auto frameStart = Clock::now();
	loop.window.handleEvent();
	loop.input.update(loop.window.hasFocus());
	if (loop.input.justPressed(client::ClientControls::QUIT)) {
		loop.logger.debug("Quit requested via input key");
		loop.window.close();
	}
	if (loop.input.justPressed(client::ClientControls::METRICS)) {
		loop.overlay.toggle();
		loop.logger.debug("Metrics overlay toggled");
	}
	if (!loop.window.isOpen()) {
		return true;
	}
	const float frameSeconds = std::chrono::duration<float>(frameStart - loop.lastFrame).count();
	const float deltaTime = std::clamp(frameSeconds, 0.0F, 0.1F);
	loop.lastFrame = frameStart;
	const std::uint8_t buttons = loop.controls.buttons(loop.input);
	loop.view.update(deltaTime, loop.client, buttons);
	reportNotices(loop.client, loop.logger);
	if (loop.client.status() != loop.previous) {
		loop.previous = loop.client.status();
		if (!reportStatus(loop.client, loop.logger)) {
			return false;
		}
	}
	loop.window.beginFrame();
	loop.view.render(loop.window, deltaTime);
	loop.overlay.draw(loop.window);
	loop.view.playAudio(loop.audio);
	// Measured before endFrame, which waits for the frame rate limit.
	const float busySeconds = std::chrono::duration<float>(Clock::now() - frameStart).count();
	loop.window.endFrame();
	loop.bus.publish(engine::core::FrameEnded{.seconds = frameSeconds, .busySeconds = busySeconds});
	return true;
}

/// Runs the graphical client until the window closes or a signal arrives.
int runClient(const ClientOptions& options)
{
	std::signal(SIGINT, stopClient);
	std::signal(SIGTERM, stopClient);

	auto& logger = engine::core::Logger::defaultLogger();

	logger.info("Starting R-Type client application...");
	engine::core::EngineSettings engineSettings;
	engine::input::InputMap bindings = client::ClientControls::defaultBindings();
	loadSettings(engineSettings, bindings, logger);

	logger.debug("Initializing core event bus and telemetry metrics...");
	engine::core::EventBus bus;
	engine::core::Metrics metrics(bus);
	engine::tools::MetricsOverlay overlay(bus);

	logger.debug("Initializing AssetManager...");
	engine::assets::AssetManager assets;

	logger.debug("Initializing network transport for " + options.host + ':' + std::to_string(options.port));
	engine::net::UdpClient transport({.host = options.host, .port = options.port, .maxDatagramSize = network::MAX_DATAGRAM_SIZE});
	network::GameClient client(transport, bus);

	logger.debug("Initializing SFML graphics backend (" + std::to_string(client_config::WINDOW_WIDTH) + "x" + std::to_string(client_config::WINDOW_HEIGHT) + ")...");
	engine::render::SfmlRenderer window({.width = client_config::WINDOW_WIDTH, .height = client_config::WINDOW_HEIGHT, .title = "R-Type"});

	logger.debug("Loading manifest games/rtype/assets/manifest.json...");
	assets.loadManifest("games/rtype/assets/manifest.json", window);
	logger.debug("Preloading asset group 'level1'...");
	assets.preload("level1", window);

	if (assets.hasFont("default")) {
		const engine::render::FontId fontId = window.loadFontFromFile(assets.getFontPath("default"));
		overlay.setFont(fontId);
		logger.debug("Default font registered for metrics overlay");
	}

	logger.debug("Initializing input and audio subsystems...");
	engine::input::SfmlInput input(std::move(bindings));
	engine::audio::SfmlAudio audio;
	client::ClientControls controls;

	logger.debug("Constructing ClientWorld view and ECS systems...");
	client::ClientWorld view(assets, engineSettings.screenFlashEnabled());

	ClientLoop loop{.window = window, .input = input, .audio = audio, .controls = controls, .client = client, .view = view, .logger = logger, .bus = bus, .overlay = overlay, .previous = client.status(), .lastFrame = Clock::now()};

	logger.info("Connecting to " + options.host + ':' + std::to_string(options.port) + "...");
	while (running != 0 && window.isOpen()) {
		if (!updateFrame(loop)) {
			logger.info("Disconnecting client from server...");
			client.disconnect();
			logger.info("Client disconnected gracefully");
			return 1;
		}
	}
	logger.info("Shutting down client application...");
	client.disconnect();
	logger.info("Client disconnected. Teardown complete.");
	return 0;
}

}
