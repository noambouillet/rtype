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

/// Polls one frame, sends the current input state, draws the frame and reports its timing to the metrics.
static bool updateFrame(ClientLoop& loop)
{
	const auto frameStart = Clock::now();
	loop.window.handleEvent();
	loop.input.update(loop.window.hasFocus());
	if (loop.input.justPressed(client::ClientControls::QUIT)) {
		loop.window.close();
	}
	if (loop.input.justPressed(client::ClientControls::METRICS)) {
		loop.overlay.toggle();
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
	auto logger = makeLogger(options.logFile);
	logger->setVerbose(true);
	engine::core::EngineSettings engineSettings;
	engine::input::InputMap bindings = client::ClientControls::defaultBindings();
	loadSettings(engineSettings, bindings);
	engine::core::EventBus bus;
	engine::core::Metrics metrics(bus);
	engine::tools::MetricsOverlay overlay(bus);
	engine::assets::AssetManager assets;
	engine::net::UdpClient transport({.host = options.host, .port = options.port, .maxDatagramSize = network::MAX_DATAGRAM_SIZE});
	network::GameClient client(transport, bus);
	engine::render::SfmlRenderer window({.width = client_config::WINDOW_WIDTH, .height = client_config::WINDOW_HEIGHT, .title = "R-Type"});
	assets.loadManifest("games/rtype/assets/manifest.json", window);
	assets.preload("level1", window);
	if (assets.hasFont("default")) {
		const engine::render::FontId fontId = window.loadFontFromFile(assets.getFontPath("default"));
		overlay.setFont(fontId);
	}
	engine::input::SfmlInput input(std::move(bindings));
	engine::audio::SfmlAudio audio;
	client::ClientControls controls;
	client::ClientWorld view(assets, engineSettings.screenFlashEnabled());
	ClientLoop loop{.window = window, .input = input, .audio = audio, .controls = controls, .client = client, .view = view, .logger = *logger, .bus = bus, .overlay = overlay, .previous = client.status(), .lastFrame = Clock::now()};
	logger->info("Connecting to " + options.host + ':' + std::to_string(options.port));
	while (running != 0 && window.isOpen()) {
		if (!updateFrame(loop)) {
			client.disconnect();
			logger->info("Disconnected");
			return 1;
		}
	}
	client.disconnect();
	logger->info("Disconnected");
	return 0;
}

}
