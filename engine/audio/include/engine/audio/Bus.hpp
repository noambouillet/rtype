/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Audio volume buses
*/

#pragma once

#include <cstddef>
#include <cstdint>

namespace engine::audio {

/// Volume group a sound plays through. Master scales every other bus.
enum class Bus : std::uint8_t { Master,
	Music,
	Sfx,
	Ui };

static constexpr std::size_t BUS_COUNT = 4;

}
