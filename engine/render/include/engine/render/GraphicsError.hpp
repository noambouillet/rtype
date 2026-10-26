/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Graphics error type
*/

#pragma once

#include <string>

#include "engine/core/Error.hpp"

namespace engine::render {

class GraphicsError : public core::Error {
public:
	explicit GraphicsError(const std::string& message) : core::Error(message)
	{
	}
};

}
