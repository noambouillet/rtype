/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML audio backend
*/

#include "engine/audio/SfmlAudio.hpp"

#include <utility>

#include "SfmlAudioImplementation.hpp"

namespace engine::audio {

/// Sound and music names are file paths relative to root, or direct paths if root is empty.
SfmlAudio::SfmlAudio(std::filesystem::path root) : _implementation(std::make_unique<Implementation>(std::move(root)))
{
}

SfmlAudio::~SfmlAudio() = default;

void SfmlAudio::playMusic(std::string_view name, float volume, Seconds fade)
{
	_implementation->playMusic(name, volume, fade);
}

void SfmlAudio::stopMusic(Seconds fade)
{
	_implementation->stopMusic(fade);
}

void SfmlAudio::playSound(std::string_view name, float volume, Bus bus, float pan)
{
	_implementation->playSound(name, volume, bus, pan);
}

void SfmlAudio::setBusVolume(Bus bus, float volume)
{
	_implementation->setBusVolume(bus, volume);
}

float SfmlAudio::busVolume(Bus bus) const
{
	return _implementation->busVolume(bus);
}

void SfmlAudio::update(float deltaTime)
{
	_implementation->update(deltaTime);
}

}
