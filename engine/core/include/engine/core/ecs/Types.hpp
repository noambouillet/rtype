/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Types
*/

#pragma once

#include <bitset>
#include <cstdint>

namespace engine::core {

constexpr std::size_t MAX_COMPONENTS = 64;
using ComponentType = std::uint8_t;
using Signature = std::bitset<MAX_COMPONENTS>;

}
