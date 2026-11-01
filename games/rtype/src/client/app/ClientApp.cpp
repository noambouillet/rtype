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
#include "client/scenes/ClientContext.hpp"
#include "client/scenes/MainMenuScene.hpp"
#include "engine/assets/AssetManager.hpp"
#include "engine/audio/SfmlAudio.hpp"
#include "engine/core/ConfigParser.hpp"
#include "engine/core/EngineSettings.hpp"
#include "engine/core/Error.hpp"
#include "engine/core/EventBus.hpp"
#include "engine/core/Logger.hpp"
#include "engine/core/metrics/Metrics.hpp"
#include "engine/core/scene/SceneManager.hpp"
#include "engine/input/SfmlInput.hpp"
#include "engine/render/SfmlRenderer.hpp"
#include "engine/tools/MetricsOverlay.hpp"

namespace rtype {

static volatile std::sig_atomic_t running = 1;

using Clock = std::chrono::steady_clock;

/// Everything one client frame reads or updates.
struct ClientLoop {
	engine::render::SfmlRenderer& window;
	engine::input::SfmlInput& input;
	engine::core::SceneManager& scenes;
	engine::core::EventBus& bus;
	engine::tools::MetricsOverlay& overlay;
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

/// Polls one frame, runs the scenes, draws the frame and reports its timing to the metrics.
static void updateFrame(ClientLoop& loop)
{
	const auto frameStart = Clock::now();
	loop.window.handleEvent();
	const engine::render::Point cursor = loop.window.cursorPosition();
	loop.input.setPointerPosition({.x = static_cast<float>(cursor.x), .y = static_cast<float>(cursor.y)});
	loop.input.update(loop.window.hasFocus());
	if (loop.input.justPressed(client::ClientControls::METRICS)) {
		loop.overlay.toggle();
	}
	if (!loop.window.isOpen()) {
		return;
	}
	const float frameSeconds = std::chrono::duration<float>(frameStart - loop.lastFrame).count();
	const float deltaTime = std::clamp(frameSeconds, 0.0F, 0.1F);
	loop.lastFrame = frameStart;
	loop.scenes.update(deltaTime);
	loop.window.beginFrame();
	loop.scenes.render();
	loop.window.setRenderSpace(engine::render::RenderSpace::Screen);
	loop.overlay.draw(loop.window);
	// Measured before endFrame, which waits for the frame rate limit.
	const float busySeconds = std::chrono::duration<float>(Clock::now() - frameStart).count();
	loop.window.endFrame();
	loop.bus.publish(engine::core::FrameEnded{.seconds = frameSeconds, .busySeconds = busySeconds});
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
	engine::render::SfmlRenderer window({.width = client_config::WINDOW_WIDTH, .height = client_config::WINDOW_HEIGHT, .title = "R-Type"});
	assets.loadManifest("games/rtype/assets/manifest.json", window);
	assets.preload("level1", window);
	engine::render::FontId font = engine::render::INVALID_FONT_ID;
	if (assets.hasFont("default")) {
		font = window.loadFontFromFile(assets.getFontPath("default"));
		overlay.setFont(font);
	}
	engine::input::SfmlInput input(std::move(bindings));
	engine::audio::SfmlAudio audio;
	engine::core::SceneManager scenes;
	client::ClientContext context{.scenes = scenes, .graphics = window, .input = input, .audio = audio, .assets = assets, .logger = *logger, .bus = bus, .options = options, .screenFlash = engineSettings.screenFlashEnabled(), .font = font};
	scenes.push(std::make_unique<client::MainMenuScene>(context));
	ClientLoop loop{.window = window, .input = input, .scenes = scenes, .bus = bus, .overlay = overlay, .lastFrame = Clock::now()};
	while (running != 0 && window.isOpen() && !scenes.empty()) {
		updateFrame(loop);
	}
	// Runs the exit code of the scenes left, such as the disconnection, while the context they use is still alive.
	scenes.clear();
	scenes.update(0.0F);
	return 0;
}

}
