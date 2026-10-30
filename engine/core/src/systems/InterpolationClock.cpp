/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** InterpolationClock
*/

#include "engine/core/systems/InterpolationClock.hpp"

#include <cmath>

namespace engine::core {

/// Creates a clock that plays a timeline delay seconds behind its latest estimate.
InterpolationClock::InterpolationClock(double delay) : _delay(delay), _sourceEstimate(0.0), _started(false)
{
}

/// Corrects the estimate of the remote timeline with a time stamp received from it, in seconds.
/// Small differences caused by network jitter are smoothed out; a large gap makes the clock jump straight to the new time.
void InterpolationClock::observe(double sourceTime)
{
	const double error = sourceTime - _sourceEstimate;

	if (!_started || std::abs(error) > SNAP_THRESHOLD) {
		_sourceEstimate = sourceTime;
		_started = true;
		return;
	}
	_sourceEstimate += error * SMOOTHING;
}

/// Moves the clock forward.
void InterpolationClock::advance(double elapsed)
{
	if (_started) {
		_sourceEstimate += elapsed;
	}
}

bool InterpolationClock::isStarted() const
{
	return _started;
}

/// Returns the current estimate of the remote time, in seconds, without the playback delay.
double InterpolationClock::sourceTime() const
{
	return _sourceEstimate;
}

/// Returns the remote time, in seconds, that should be shown on screen now.
double InterpolationClock::renderTime() const
{
	return _sourceEstimate - _delay;
}

}
