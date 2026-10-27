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
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "client/SFML/Sfml.hpp"
#include "client/SFML/SoundPlayer.hpp"
#include "client/network/GameClient.hpp"
#include "client/network/UdpClient.hpp"
#include "client/view/ClientWorld.hpp"
#include "engine/core/Logger.hpp"
#include "engine/render/AssetManager.hpp"

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

/// Polls one frame and sends the current input state.
static bool updateFrame(Sfml& window, SoundPlayer& audio, network::GameClient& client, rtype::client::ClientWorld& view,
	engine::core::Logger& logger, network::ClientStatus& previous, std::chrono::steady_clock::time_point& lastFrame,
	engine::render::AssetManager& assets)
{
	window.handleEvent();
	if (!window.isOpen()) {
		return true;
	}
	assets.update(window);
	const auto now = std::chrono::steady_clock::now();
	const float deltaTime = std::clamp(std::chrono::duration<float>(now - lastFrame).count(), 0.0F, 0.1F);
	lastFrame = now;
	const std::uint8_t buttons = window.buttons();
	view.update(client, buttons, deltaTime);
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

/// Resolves the path to the game manifest file.
static std::string resolveClientManifestPath()
{
	const std::array<std::string, 3> candidates = {
		"assets/manifest.json",
		"games/rtype/assets/manifest.json",
		"../assets/manifest.json"};
	for (const auto& candidate : candidates) {
		if (std::filesystem::exists(candidate)) {
			return candidate;
		}
	}
	return "assets/manifest.json";
}

/// Runs the main rendering and event loop for the client.
static bool runClientLoop(Sfml& window, SoundPlayer& audio, network::GameClient& client,
	rtype::client::ClientWorld& view, engine::core::Logger& logger, engine::render::AssetManager& assets)
{
	auto previous = client.status();
	auto lastFrame = std::chrono::steady_clock::now();
	while (running != 0 && window.isOpen()) {
		if (!updateFrame(window, audio, client, view, logger, previous, lastFrame, assets)) {
			return false;
		}
	}
	return true;
}

/// Runs the graphical client until the window closes or a signal arrives.
int runClient(const ClientOptions& options)
{
	std::signal(SIGINT, stopClient);
	std::signal(SIGTERM, stopClient);
	auto logger = makeLogger(options.logFile);
	logger->setVerbose(true);
	engine::render::AssetManager assets;
	network::UdpClient transport(options.host, options.port);
	network::GameClient client(transport);
	Sfml window(assets);
	assets.loadManifest(resolveClientManifestPath(), window);
	SoundPlayer audio;
	rtype::client::ClientWorld view(assets);
	logger->info("Connecting to " + options.host + ':' + std::to_string(options.port));
	const bool success = runClientLoop(window, audio, client, view, *logger, assets);
	client.disconnect();
	logger->info("Disconnected");
	return success ? 0 : 1;
}

}
