/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Screen placement of interface elements
*/

#pragma once

#include "engine/render/GraphicsBackend.hpp"
#include "engine/ui/Components.hpp"

namespace engine::ui {

struct UiBounds {
	float x;
	float y;
	float width;
	float height;
};

UiBounds resolveBounds(const UiTransform& transform, render::Point screen);
bool contains(const UiBounds& bounds, float x, float y);

}
