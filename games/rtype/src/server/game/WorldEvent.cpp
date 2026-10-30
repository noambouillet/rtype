/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Gameplay event produced by one server world tick
*/

#include "WorldEvent.hpp"

namespace rtype {

WorldEvent::WorldEvent(Type type, ecs::ReplicatedState entity) : _type(type), _entity(entity)
{
}

WorldEvent::Type WorldEvent::type() const
{
	return _type;
}

const ecs::ReplicatedState& WorldEvent::entity() const
{
	return _entity;
}

}
