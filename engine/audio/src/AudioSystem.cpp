/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Audio system
*/

#include "engine/audio/AudioSystem.hpp"

#include <utility>
#include <vector>

#include "engine/audio/Components.hpp"

namespace engine::audio {

AudioSystem::AudioSystem() : _music{}, _musicVolume(0.0F), _musicFade(0.0F), _musicStarted(false)
{
}

/// Sets the looping music started on the next play, faded in over fade.
void AudioSystem::setMusic(std::string name, float volume, Seconds fade)
{
	_music = std::move(name);
	_musicVolume = volume;
	_musicFade = fade;
	_musicStarted = false;
}

core::Signature AudioSystem::signature(core::Coordinator& coordinator)
{
	core::Signature signature;
	signature.set(coordinator.getComponentType<Sound>());
	return signature;
}

/// Advances the backend fades, then plays and destroys every pending sound entity.
void AudioSystem::play(core::Coordinator& coordinator, IAudioBackend& backend, float deltaTime)
{
	backend.update(deltaTime);
	if (!_musicStarted && !_music.empty()) {
		backend.playMusic(_music, _musicVolume, _musicFade);
		_musicStarted = true;
	}
	const std::vector<core::Entity> played(entities.begin(), entities.end());
	for (core::Entity entity : played) {
		const Sound& sound = coordinator.getComponent<Sound>(entity);
		backend.playSound(sound.name, sound.volume, sound.bus, sound.pan);
		coordinator.destroyEntity(entity);
	}
}

}
