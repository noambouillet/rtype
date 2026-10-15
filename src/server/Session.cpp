/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Per-player server session implementation
*/

#include "Session.hpp"

#include <utility>

#include "utils/network/Protocol.hpp"

namespace rtype {

Session::Session(network::Endpoint address, std::uint64_t nonce, Clock::time_point now, std::uint32_t id)
	: _address(std::move(address)), _nonce(nonce), _id(id), _nextSequence(1), _lastInputSequence(0), _lastHeard(now), _lastInput(now), _buttons(0), _hasInput(false)
{
}

const network::Endpoint& Session::address() const
{
	return _address;
}

std::uint64_t Session::nonce() const
{
	return _nonce;
}

std::uint32_t Session::id() const
{
	return _id;
}

/// Records the latest valid packet's source as this session's reply address.
void Session::refresh(network::Endpoint address, Clock::time_point now)
{
	_address = std::move(address);
	_lastHeard = now;
}

/// Accepts a button state only when its sequence is newer than the last input.
void Session::acceptInput(Input input, Clock::time_point now)
{
	if (_hasInput && !network::isNewerSequence(input.sequence, _lastInputSequence)) {
		return;
	}
	_buttons = input.buttons;
	_lastInputSequence = input.sequence;
	_lastInput = now;
	_hasInput = true;
}

/// Returns the next outbound sequence and skips zero after wraparound.
std::uint32_t Session::takeNextSequence()
{
	const std::uint32_t sequence = _nextSequence++;
	if (_nextSequence == 0) {
		_nextSequence = 1;
	}
	return sequence;
}

/// Reports whether no valid client packet arrived within "timeout".
bool Session::hasExpired(Clock::time_point now, Clock::duration timeout) const
{
	return now - _lastHeard >= timeout;
}

/// Returns current buttons, or zero when input is absent or stale.
std::uint8_t Session::buttonsAt(Clock::time_point now, Clock::duration timeout) const
{
	if (!_hasInput || now - _lastInput >= timeout) {
		return 0;
	}
	return _buttons;
}

}
