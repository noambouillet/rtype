/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Typed publish/subscribe event bus
*/

#include "engine/core/EventBus.hpp"

#include <algorithm>
#include <utility>

namespace engine::core {

Subscription::Subscription() : _bus(nullptr), _id(0)
{
}

Subscription::Subscription(EventBus& bus, std::size_t id) : _bus(&bus), _id(id)
{
}

Subscription::~Subscription()
{
	reset();
}

Subscription::Subscription(Subscription&& other) noexcept : _bus(std::exchange(other._bus, nullptr)), _id(other._id)
{
}

Subscription& Subscription::operator=(Subscription&& other) noexcept
{
	if (this != &other) {
		reset();
		_bus = std::exchange(other._bus, nullptr);
		_id = other._id;
	}
	return *this;
}

/// Removes the handler now instead of waiting for destruction.
void Subscription::reset()
{
	if (_bus != nullptr) {
		_bus->unsubscribe(_id);
		_bus = nullptr;
	}
}

EventBus::EventBus() : _nextId(1), _dispatchDepth(0), _hasInactive(false)
{
}

/// Removes a handler. During a dispatch it is only disabled, then erased once the dispatch ends.
void EventBus::unsubscribe(std::size_t id)
{
	std::erase_if(_pending, [id](const PendingHandler& pending) {
		return pending.handler.id == id;
	});
	for (auto& [type, handlers] : _handlers) {
		const auto it = std::ranges::find(handlers, id, &Handler::id);
		if (it == handlers.end()) {
			continue;
		}
		if (_dispatchDepth > 0) {
			it->active = false;
			_hasInactive = true;
		} else {
			handlers.erase(it);
		}
		return;
	}
}

/// Applies the subscriptions changed while handlers were running.
void EventBus::flushPending()
{
	if (_hasInactive) {
		for (auto& [type, handlers] : _handlers) {
			std::erase_if(handlers, [](const Handler& handler) {
				return !handler.active;
			});
		}
		_hasInactive = false;
	}
	for (PendingHandler& pending : _pending) {
		_handlers[pending.type].push_back(std::move(pending.handler));
	}
	_pending.clear();
}

}
