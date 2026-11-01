/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Screen placement of interface elements
*/

#include "engine/ui/Layout.hpp"

namespace engine::ui {

/// Gives where the anchor sits horizontally: 0 on the left edge, 0.5 in the middle, 1 on the right edge.
static float horizontalFactor(Anchor anchor)
{
	switch (anchor) {
		case Anchor::Top:
		case Anchor::Center:
		case Anchor::Bottom:
			return 0.5F;
		case Anchor::TopRight:
		case Anchor::Right:
		case Anchor::BottomRight:
			return 1.0F;
		default:
			return 0.0F;
	}
}

/// Gives where the anchor sits vertically: 0 on the top edge, 0.5 in the middle, 1 on the bottom edge.
static float verticalFactor(Anchor anchor)
{
	switch (anchor) {
		case Anchor::Left:
		case Anchor::Center:
		case Anchor::Right:
			return 0.5F;
		case Anchor::BottomLeft:
		case Anchor::Bottom:
		case Anchor::BottomRight:
			return 1.0F;
		default:
			return 0.0F;
	}
}

/// Places an element on the screen. The anchor is both the screen point and the point of the element that sits on it
UiBounds resolveBounds(const UiTransform& transform, render::Point screen)
{
	const float factorX = horizontalFactor(transform.anchor);
	const float factorY = verticalFactor(transform.anchor);
	return {
		.x = (factorX * static_cast<float>(screen.x)) + transform.offsetX - (factorX * transform.width),
		.y = (factorY * static_cast<float>(screen.y)) + transform.offsetY - (factorY * transform.height),
		.width = transform.width,
		.height = transform.height,
	};
}

/// Tells whether a point is inside the bounds. The right and bottom edges are outside, so two touching elements never both contain a point.
bool contains(const UiBounds& bounds, float x, float y)
{
	return x >= bounds.x && x < bounds.x + bounds.width && y >= bounds.y && y < bounds.y + bounds.height;
}

}
