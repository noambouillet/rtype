/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Project error types
*/

#pragma once

#include <string>

#include "engine/core/Error.hpp"

namespace rtype {

class RTypeError : public engine::core::Error {
public:
	explicit RTypeError(const std::string& message) : Error(message)
	{
	}
};

class NetworkError : public RTypeError {
public:
	explicit NetworkError(const std::string& message) : RTypeError(message)
	{
	}
};

class GraphicsError : public RTypeError {
public:
	explicit GraphicsError(const std::string& message) : RTypeError(message)
	{
	}
};

class AudioError : public RTypeError {
public:
	explicit AudioError(const std::string& message) : RTypeError(message)
	{
	}
};

}
