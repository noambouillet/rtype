/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client view world
*/

#include "ClientWorld.hpp"

#include <optional>
#include <string>
#include <vector>

#include "client/view/ClientComponents.hpp"
#include "client/view/ClientNetworkSystem.hpp"
#include "client/view/EffectSystem.hpp"
#include "client/view/SoundEventSystem.hpp"
#include "engine/audio/AudioSystem.hpp"
#include "engine/audio/Components.hpp"
#include "engine/core/ecs/Components.hpp"
#include "engine/render/Components.hpp"
#include "engine/render/RenderSystem.hpp"

namespace rtype::client {

ClientWorld::ClientWorld()
{
	_coordinator.registerComponent<engine::core::Position>();
	_coordinator.registerComponent<engine::render::Sprite>();
	_coordinator.registerComponent<engine::render::Animation>();
	_coordinator.registerComponent<NetworkIdentity>();
	_coordinator.registerComponent<Effect>();
	_coordinator.registerComponent<engine::audio::Sound>();
	auto& network = _coordinator.registerSystem<ClientNetworkSystem>();
	_coordinator.setSystemSignature<ClientNetworkSystem>(network.signature(_coordinator));
	auto& effects = _coordinator.registerSystem<EffectSystem>();
	_coordinator.setSystemSignature<EffectSystem>(effects.signature(_coordinator));
	auto& render = _coordinator.registerSystem<engine::render::RenderSystem>();
	_coordinator.setSystemSignature<engine::render::RenderSystem>(render.signature(_coordinator));
	render.setBackground("background.png", 40.0F);
	auto& sounds = _coordinator.registerSystem<SoundEventSystem>();
	_coordinator.setSystemSignature<SoundEventSystem>(sounds.signature(_coordinator));
	auto& audio = _coordinator.registerSystem<engine::audio::AudioSystem>();
	_coordinator.setSystemSignature<engine::audio::AudioSystem>(audio.signature(_coordinator));
	audio.setMusic(std::string(SoundEventSystem::MUSIC), SoundEventSystem::MUSIC_VOLUME);
}

/// Updates the network copy and temporary presentation state for one client frame.
void ClientWorld::update(network::GameClient& client, std::uint8_t buttons, float deltaTime)
{
	auto& network = _coordinator.getSystem<ClientNetworkSystem>();
	auto& effects = _coordinator.getSystem<EffectSystem>();
	auto& sounds = _coordinator.getSystem<SoundEventSystem>();
	const std::vector<ViewEvent> events = network.update(_coordinator, client, buttons);
	const std::optional<std::uint8_t> playerId = client.status() == network::ClientStatus::Connected ? std::optional<std::uint8_t>(client.playerId()) : std::nullopt;
	effects.consume(_coordinator, network, events, playerId);
	sounds.consume(_coordinator, network, events, playerId);
	if (effects.predictLocalFire(_coordinator, network, buttons, playerId)) {
		sounds.localFire(_coordinator);
	}
	effects.update(_coordinator, network, deltaTime);
}

void ClientWorld::render(engine::render::IGraphicsBackend& backend, float deltaTime)
{
	_coordinator.getSystem<engine::render::RenderSystem>().render(_coordinator, backend, deltaTime);
}

void ClientWorld::playAudio(engine::audio::IAudioBackend& backend)
{
	_coordinator.getSystem<engine::audio::AudioSystem>().play(_coordinator, backend);
}

}
