/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** PlayerControlSystem
*/

#include "PlayerControlSystem.hpp"

#include <cmath>

#include "engine/core/ecs/Components.hpp"
#include "game/GameTypes.hpp"
#include "server/ecs/GameComponents.hpp"

namespace rtype::ecs {

/// Builds the signature the system needs (PlayerControl + Velocity + Weapon): the ships driven by a client.
engine::core::Signature PlayerControlSystem::signature(engine::core::Coordinator& coordinator)
{
	engine::core::Signature sig;

	sig.set(coordinator.getComponentType<PlayerControl>());
	sig.set(coordinator.getComponentType<engine::core::Velocity>());
	sig.set(coordinator.getComponentType<Weapon>());
	return sig;
}

/// Turns each player's buttons into a velocity (diagonals normalised) and a fire request for WeaponSystem.
void PlayerControlSystem::update(engine::core::Coordinator& coordinator, const Buttons& buttons)
{
	for (const engine::core::Entity& e : entities) {
		const auto& control = coordinator.getComponent<PlayerControl>(e);
		const std::uint8_t pressed = control.playerId < buttons.size() ? buttons[control.playerId] : 0;
		const float horizontal = static_cast<float>((pressed & game::BUTTON_RIGHT) != 0) - static_cast<float>((pressed & game::BUTTON_LEFT) != 0);
		const float vertical = static_cast<float>((pressed & game::BUTTON_DOWN) != 0) - static_cast<float>((pressed & game::BUTTON_UP) != 0);
		const float length = std::sqrt((horizontal * horizontal) + (vertical * vertical));

		auto& velocity = coordinator.getComponent<engine::core::Velocity>(e);
		if (length == 0.0F) {
			velocity = {.x = 0.0F, .y = 0.0F};
		} else {
			velocity = {.x = control.speed * horizontal / length, .y = control.speed * vertical / length};
		}
		coordinator.getComponent<Weapon>(e).firing = (pressed & game::BUTTON_FIRE) != 0;
	}
}

}
