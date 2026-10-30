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
#include "engine/core/systems/InterpolationSystem.hpp"
#include "engine/render/Components.hpp"
#include "engine/render/RenderSystem.hpp"

namespace rtype::client {

/// Builds the view world. With screenFlash off, explosions are never shown (accessibility setting).
ClientWorld::ClientWorld(bool screenFlash)
{
	_coordinator.registerComponent<engine::core::Position>();
	_coordinator.registerComponent<engine::core::Interpolated>();
	_coordinator.registerComponent<engine::render::Sprite>();
	_coordinator.registerComponent<engine::render::Animation>();
	_coordinator.registerComponent<NetworkIdentity>();
	_coordinator.registerComponent<Effect>();
	_coordinator.registerComponent<engine::audio::Sound>();
	_coordinator.registerSystem<ClientNetworkSystem>();
	_coordinator.setSystemSignature<ClientNetworkSystem>(ClientNetworkSystem::signature(_coordinator));
	_coordinator.registerSystem<engine::core::InterpolationSystem>();
	_coordinator.setSystemSignature<engine::core::InterpolationSystem>(engine::core::InterpolationSystem::signature(_coordinator));
	auto& effects = _coordinator.registerSystem<EffectSystem>();
	_coordinator.setSystemSignature<EffectSystem>(EffectSystem::signature(_coordinator));
	effects.setScreenFlash(screenFlash);
	auto& render = _coordinator.registerSystem<engine::render::RenderSystem>();
	_coordinator.setSystemSignature<engine::render::RenderSystem>(engine::render::RenderSystem::signature(_coordinator));
	render.setBackground("background.png", 40.0F);
	_coordinator.registerSystem<SoundEventSystem>();
	_coordinator.setSystemSignature<SoundEventSystem>(SoundEventSystem::signature(_coordinator));
	auto& audio = _coordinator.registerSystem<engine::audio::AudioSystem>();
	_coordinator.setSystemSignature<engine::audio::AudioSystem>(engine::audio::AudioSystem::signature(_coordinator));
	audio.setMusic(std::string(SoundEventSystem::MUSIC), SoundEventSystem::MUSIC_VOLUME);
}

/// Updates the network copy and temporary presentation state for one client frame.
void ClientWorld::update(float deltaTime, network::GameClient& client, std::uint8_t buttons)
{
	auto& network = _coordinator.getSystem<ClientNetworkSystem>();
	auto& effects = _coordinator.getSystem<EffectSystem>();
	auto& sounds = _coordinator.getSystem<SoundEventSystem>();
	const std::vector<ViewEvent> events = network.update(deltaTime, _coordinator, client, buttons);
	_coordinator.getSystem<engine::core::InterpolationSystem>().update(_coordinator, network.renderTime());
	const std::optional<std::uint8_t> playerId = client.status() == network::ClientStatus::Connected ? std::optional<std::uint8_t>(client.playerId()) : std::nullopt;
	effects.consume(_coordinator, network, events, playerId);
	sounds.consume(_coordinator, network, events, playerId);
	if (effects.predictLocalFire(_coordinator, network, buttons, playerId)) {
		rtype::client::SoundEventSystem::localFire(_coordinator);
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
