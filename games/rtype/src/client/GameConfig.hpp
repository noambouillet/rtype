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
static constexpr std::string_view SETTINGS_FILE = "settings.cfg";
static constexpr std::string_view FONT_FILE = "fonts/LiberationMono-Regular.ttf";
static constexpr float OFFSCREEN_MARGIN = 96.0F;  // matches the server margin past which entities are removed

}
