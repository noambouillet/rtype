/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Queue of items released when their time comes
*/

#pragma once

#include <cstddef>
#include <map>
#include <optional>
#include <vector>

namespace engine::core {

struct TimedQueueLimits {
	std::size_t maxSize;
	double maxLateness;
};

template <typename T>
class TimedQueue {
public:
	explicit TimedQueue(TimedQueueLimits limits);
	bool push(double time, T item);
	std::vector<T> release(double now);
	void clear();
	[[nodiscard]] std::size_t size() const;
	[[nodiscard]] bool empty() const;

private:
	std::multimap<double, T> _items;
	std::optional<double> _lastRelease;
	TimedQueueLimits _limits;
};

}

#include "engine/core/TimedQueue.tpp"
