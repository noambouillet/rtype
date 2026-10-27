/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Typed publish/subscribe event bus
*/

#pragma once

#include <utility>

namespace engine::core {

/// Registers a handler called on every published Event of this exact type.
/// A handler added while an event is dispatched only receives the next ones.
template <typename Event>
Subscription EventBus::subscribe(std::function<void(const Event&)> handler)
{
	const std::size_t id = _nextId++;
	Handler entry{id, [callback = std::move(handler)](const void* event) {
		callback(*static_cast<const Event*>(event));
	}, true};

	if (_dispatchDepth > 0) {
		_pending.push_back({std::type_index(typeid(Event)), std::move(entry)});
	} else {
		_handlers[std::type_index(typeid(Event))].push_back(std::move(entry));
	}
	return Subscription(*this, id);
}

/// Calls every active handler subscribed to this event type, in subscription order.
template <typename Event>
void EventBus::publish(const Event& event)
{
	const auto it = _handlers.find(std::type_index(typeid(Event)));
	if (it == _handlers.end()) {
		return;
	}
	struct DispatchGuard {
		EventBus& bus;
		~DispatchGuard()
		{
			if (--bus._dispatchDepth == 0) {
				bus.flushPending();
			}
		}
	};

	++_dispatchDepth;
	const DispatchGuard guard{*this};
	for (const Handler& handler : it->second) {
		if (handler.active) {
			handler.callback(&event);
		}
	}
}

}
