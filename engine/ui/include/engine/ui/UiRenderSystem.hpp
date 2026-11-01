/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Interface render system
*/

#pragma once

#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"
#include "engine/render/GraphicsBackend.hpp"
#include "engine/ui/Layout.hpp"

namespace engine::ui {

class UiRenderSystem : public core::System {
public:
	static core::Signature signature(core::Coordinator& coordinator);
	void render(core::Coordinator& coordinator, render::IGraphicsBackend& backend);

private:
	static void drawImage(const UiImage& image, const UiBounds& bounds, render::IGraphicsBackend& backend);
	static void drawText(const UiText& text, const UiBounds& bounds, render::IGraphicsBackend& backend);
};

}
