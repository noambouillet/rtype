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

Subscription::Subscription() : _bus(std::nullopt), _id(0)
{
}

Subscription::Subscription(EventBus& bus, std::size_t id) : _bus(bus), _id(id)
{
}

Subscription::~Subscription()
{
	reset();
}

Subscription::Subscription(Subscription&& other) noexcept : _bus(std::exchange(other._bus, std::nullopt)), _id(other._id)
{
}

Subscription& Subscription::operator=(Subscription&& other) noexcept
{
	if (this != &other) {
		reset();
		_bus = std::exchange(other._bus, std::nullopt);
		_id = other._id;
	}
	return *this;
}

/// Removes the handler now instead of waiting for destruction.
void Subscription::reset()
{
	if (_bus.has_value()) {
		_bus->get().unsubscribe(_id);
		_bus.reset();
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
		const auto target = std::ranges::find(handlers, id, &Handler::id);
		if (target == handlers.end()) {
			continue;
		}
		if (_dispatchDepth > 0) {
			target->active = false;
			_hasInactive = true;
		} else {
			handlers.erase(target);
		}
		return;
	}
}

/// Marks the bus as dispatching for the guard's lifetime, so subscription
/// changes made by handlers are deferred until the outermost dispatch ends.
EventBus::DispatchGuard::DispatchGuard(EventBus& bus) : _bus(bus)
{
	_bus._dispatchDepth++;
}

EventBus::DispatchGuard::~DispatchGuard()
{
	_bus._dispatchDepth--;
	if (_bus._dispatchDepth == 0) {
		_bus.flushPending();
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
