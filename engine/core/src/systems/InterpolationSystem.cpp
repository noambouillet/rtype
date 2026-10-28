/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** InterpolationSystem
*/

#include "engine/core/systems/InterpolationSystem.hpp"

#include <algorithm>
#include <vector>

namespace engine::core {

/// Builds the signature the system needs (Position + Interpolated).
Signature InterpolationSystem::signature(Coordinator& coordinator)
{
	Signature sig;

	sig.set(coordinator.getComponentType<Position>());
	sig.set(coordinator.getComponentType<Interpolated>());
	return sig;
}

/// Records a timed position, keeping samples sorted by time.
/// A sample older than the newest one is ignored, and the oldest samples are dropped once MAX_SAMPLES is reached.
void InterpolationSystem::addSample(Interpolated& interpolated, const PositionSample& sample)
{
	std::vector<PositionSample>& samples = interpolated.samples;

	if (!samples.empty() && sample.time <= samples.back().time) {
		return;
	}
	if (samples.size() >= MAX_SAMPLES) {
		samples.erase(samples.begin());
	}
	samples.push_back(sample);
}

/// Returns the position on the straight line between "from" and "to" at a certain time.
static Position blend(const PositionSample& from, const PositionSample& to, double time)
{
	const auto ratio = static_cast<float>((time - from.time) / (to.time - from.time));

	return Position{.x = from.x + ((to.x - from.x) * ratio), .y = from.y + ((to.y - from.y) * ratio)};
}

/// Places every entity where its samples say it was at renderTime
/// Samples that can no longer be needed are dropped.
void InterpolationSystem::update(Coordinator& coordinator, double renderTime)
{
	for (const Entity& e : entities) {
		auto& samples = coordinator.getComponent<Interpolated>(e).samples;
		if (samples.empty()) {
			continue;
		}
		const auto next = std::ranges::find_if(samples, [renderTime](const PositionSample& sample) { return sample.time > renderTime; });
		auto& position = coordinator.getComponent<Position>(e);
		if (next == samples.begin()) {
			position = Position{.x = next->x, .y = next->y};
			continue;
		}
		if (next == samples.end()) {
			position = Position{.x = samples.back().x, .y = samples.back().y};
			samples.erase(samples.begin(), samples.end() - 1);
			continue;
		}
		position = blend(*(next - 1), *next, renderTime);
		samples.erase(samples.begin(), next - 1);
	}
}

}
