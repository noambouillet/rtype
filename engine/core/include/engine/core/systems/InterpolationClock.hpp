/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** InterpolationClock
*/

#pragma once

namespace engine::core {

class InterpolationClock {
private:
	double _delay;
	double _sourceEstimate;
	bool _started;

public:
	static constexpr double SMOOTHING = 0.1;
	static constexpr double SNAP_THRESHOLD = 0.25;

	explicit InterpolationClock(double delay);
	void observe(double sourceTime);
	void advance(double elapsed);
	[[nodiscard]] bool isStarted() const;
	[[nodiscard]] double sourceTime() const;
	[[nodiscard]] double renderTime() const;
};

}
