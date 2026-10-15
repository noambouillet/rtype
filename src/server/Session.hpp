/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Per-player server session
*/

#pragma once

#include <chrono>
#include <cstdint>

#include "game_engine/Network/Transport.hpp"

namespace rtype {

class Session {
private:
	using Clock = std::chrono::steady_clock;

	network::Endpoint _address;
	std::uint64_t _nonce;
	std::uint32_t _id;
	std::uint32_t _nextSequence;
	std::uint32_t _lastInputSequence;
	Clock::time_point _lastHeard;
	Clock::time_point _lastInput;
	std::uint8_t _buttons;
	bool _hasInput;

public:
	struct Input {
		std::uint32_t sequence;
		std::uint8_t buttons;
	};

	Session(network::Endpoint address, std::uint64_t nonce, Clock::time_point now, std::uint32_t id);

	[[nodiscard]] const network::Endpoint& address() const;
	[[nodiscard]] std::uint64_t nonce() const;
	[[nodiscard]] std::uint32_t id() const;
	void refresh(network::Endpoint address, Clock::time_point now);
	void acceptInput(Input input, Clock::time_point now);
	std::uint32_t takeNextSequence();
	[[nodiscard]] bool hasExpired(Clock::time_point now, Clock::duration timeout) const;
	[[nodiscard]] std::uint8_t buttonsAt(Clock::time_point now, Clock::duration timeout) const;
};

}
