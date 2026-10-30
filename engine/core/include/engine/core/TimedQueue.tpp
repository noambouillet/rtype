/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Queue of items released when their time comes
*/

#pragma once

#include <iterator>
#include <utility>

namespace engine::core {

/// Creates an empty queue.
/// Beyond maxSize items, the oldest come out early.
/// Items more than maxLateness seconds older than the last release are refused.
template <typename T>
TimedQueue<T>::TimedQueue(TimedQueueLimits limits) : _items{}, _lastRelease{}, _limits(limits)
{
}

/// Schedules an item for a time in seconds.
/// Returns false and drops it if it is already too late.
template <typename T>
bool TimedQueue<T>::push(double time, T item)
{
	if (_lastRelease && time < *_lastRelease - _limits.maxLateness) {
		return false;
	}
	_items.emplace(time, std::move(item));
	return true;
}

/// Takes out every item due by now, earliest first.
/// Items with the same time keep their push order.
template <typename T>
std::vector<T> TimedQueue<T>::release(double now)
{
	_lastRelease = now;
	const std::size_t overflow = _items.size() > _limits.maxSize ? _items.size() - _limits.maxSize : 0;
	auto end = _items.upper_bound(now);
	if (static_cast<std::size_t>(std::distance(_items.begin(), end)) < overflow) {
		end = std::next(_items.begin(), static_cast<std::ptrdiff_t>(overflow));
	}
	std::vector<T> released;
	released.reserve(static_cast<std::size_t>(std::distance(_items.begin(), end)));
	for (auto entry = _items.begin(); entry != end; entry++) {
		released.push_back(std::move(entry->second));
	}
	_items.erase(_items.begin(), end);
	return released;
}

/// Drops every waiting item and forgets the last release time.
template <typename T>
void TimedQueue<T>::clear()
{
	_items.clear();
	_lastRelease.reset();
}

template <typename T>
std::size_t TimedQueue<T>::size() const
{
	return _items.size();
}

template <typename T>
bool TimedQueue<T>::empty() const
{
	return _items.empty();
}

}
