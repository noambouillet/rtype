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

namespace rtype::client_config {

static constexpr std::size_t MAX_PLAYERS = 4;
static constexpr std::uint32_t WINDOW_WIDTH = 1280;
static constexpr std::uint32_t WINDOW_HEIGHT = 720;
static constexpr auto FIRE_INTERVAL = std::chrono::milliseconds(250);
static constexpr std::string_view BINDINGS_FILE = "bindings.cfg";

}
