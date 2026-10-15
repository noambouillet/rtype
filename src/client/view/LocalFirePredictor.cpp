/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Timing for the local firing effect
*/

#include "LocalFirePredictor.hpp"

#include "client/GameConfig.hpp"

namespace rtype {

LocalFirePredictor::LocalFirePredictor() : _nextShot(TimePoint::min())
{
}

/// Returns whether to play a cute weapon burst at the local cadence.
bool LocalFirePredictor::update(bool firing, bool active, TimePoint now)
{
	if (!active) {
		_nextShot = TimePoint::min();
		return false;
	}
	if (!firing || now < _nextShot) {
		return false;
	}
	_nextShot = now + client_config::FIRE_INTERVAL;
	return true;
}

}
