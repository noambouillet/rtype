/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Audio error type
*/

#pragma once

#include <string>

#include "engine/core/Error.hpp"

namespace engine::audio {

class AudioError : public core::Error {
public:
	explicit AudioError(const std::string& message) : core::Error(message)
	{
	}
};

}
