/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Asset error type
*/

#pragma once

#include <string>

#include "engine/core/Error.hpp"

namespace engine::assets {

class AssetError : public core::Error {
public:
	explicit AssetError(const std::string& message) : core::Error(message)
	{
	}
};

}
