/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Typed publish/subscribe event bus
*/

#pragma once

#include <typeinfo>
#include <utility>

namespace engine::core {

/// Registers a handler called on every published Event of this exact type.
/// A handler added while an event is dispatched only receives the next ones.
template <typename Event>
Subscription EventBus::subscribe(std::function<void(const Event&)> handler)
{
	const std::size_t id = _nextId++;
	auto callback = [eventHandler = std::move(handler)](const std::any& event) {
		eventHandler(std::any_cast<std::reference_wrapper<const Event>>(event).get());
	};
	Handler entry{id, std::move(callback), true};

	if (_dispatchDepth > 0) {
		_pending.push_back({std::type_index(typeid(Event)), std::move(entry)});
	} else {
		_handlers[std::type_index(typeid(Event))].push_back(std::move(entry));
	}
	return {*this, id};
}

/// Calls every active handler subscribed to this event type, in subscription order.
template <typename Event>
void EventBus::publish(const Event& event)
{
	const auto typeHandlers = _handlers.find(std::type_index(typeid(Event)));
	if (typeHandlers == _handlers.end()) {
		return;
	}
	const DispatchGuard guard(*this);
	for (const Handler& handler : typeHandlers->second) {
		if (handler.active) {
			handler.callback(std::cref(event));
		}
	}
}

}
