/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Events published on the EventBus to feed the metrics collector
*/

#pragma once

#include <cstddef>

namespace engine::core {

struct FrameEnded {
	float seconds;
	float busySeconds;
};

struct BytesSent {
	std::size_t count;
};

struct BytesReceived {
	std::size_t count;
};

struct PingSent {
};

struct RoundTripMeasured {
	float milliseconds;
};

}
