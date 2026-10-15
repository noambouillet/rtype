/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Types
*/

#pragma once

#include <bitset>
#include <cstdint>

namespace rtype::ecs {

constexpr std::size_t MAX_COMPONENTS = 64;
using ComponentType = std::uint8_t;
using Signature = std::bitset<MAX_COMPONENTS>;

}
