/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Network error type
*/

#pragma once

#include <string>

#include "engine/core/Error.hpp"

namespace engine::net {

class NetworkError : public core::Error {
public:
	explicit NetworkError(const std::string& message) : core::Error(message)
	{
	}
};

}
