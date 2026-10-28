/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client presentation rules
*/

#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "utils/network/Protocol.hpp"

namespace rtype::client_config {

static constexpr std::size_t MAX_PLAYERS = 4;
static constexpr std::uint32_t WINDOW_WIDTH = 1280;
static constexpr std::uint32_t WINDOW_HEIGHT = 720;
static constexpr auto FIRE_INTERVAL = std::chrono::milliseconds(250);
static constexpr std::string_view SETTINGS_FILE = "settings.cfg";
// Seconds the view plays behind the server: 2.5 snapshot intervals.
static constexpr double INTERPOLATION_DELAY = 2.5 * static_cast<double>(network::SNAPSHOT_TICKS) * network::TICK_SECONDS;
static constexpr float OFFSCREEN_MARGIN = 96.0F;  // matches the server margin past which entities are removed

}
