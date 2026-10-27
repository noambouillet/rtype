/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Typed publish/subscribe event bus
*/

#pragma once

#include <cstddef>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace engine::core {

class EventBus;

/// Keeps a handler registered while alive and removes it when destroyed.
/// The bus it comes from must outlive it.
class Subscription {
public:
	Subscription();
	Subscription(EventBus& bus, std::size_t id);
	~Subscription();

	Subscription(const Subscription&) = delete;
	Subscription& operator=(const Subscription&) = delete;
	Subscription(Subscription&& other) noexcept;
	Subscription& operator=(Subscription&& other) noexcept;

	void reset();

private:
	EventBus* _bus;
	std::size_t _id;
};

class EventBus {
public:
	EventBus();
	~EventBus() = default;

	EventBus(const EventBus&) = delete;
	EventBus& operator=(const EventBus&) = delete;

	template <typename Event>
	[[nodiscard]] Subscription subscribe(std::function<void(const Event&)> handler);

	template <typename Event>
	void publish(const Event& event);

	void unsubscribe(std::size_t id);

private:
	struct Handler {
		std::size_t id;
		std::function<void(const void*)> callback;
		bool active;
	};

	struct PendingHandler {
		std::type_index type;
		Handler handler;
	};

	void flushPending();

	std::unordered_map<std::type_index, std::vector<Handler>> _handlers;
	std::vector<PendingHandler> _pending;
	std::size_t _nextId;
	std::size_t _dispatchDepth;
	bool _hasInactive;
};

}

#include "engine/core/EventBus.tpp"
