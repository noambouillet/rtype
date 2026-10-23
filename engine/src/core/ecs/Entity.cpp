/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Entity
*/

#include "Entity.hpp"

namespace rtype::ecs {

/// Builds the null entity, which never refers to a living entity.
Entity::Entity() : _id(NULL_ID)
{
}

/// Packs the generation (high bits) and the index (low bits) in one 32-bit id.
Entity::Entity(Index index, Generation generation)
	: _id((static_cast<Id>(generation) << INDEX_BITS) | (index & INDEX_MASK))
{
}

/// Returns the packed 32-bit id.
Entity::Id Entity::id() const
{
	return _id;
}

/// Returns the slot number of the entity, used to index sparse sets.
Entity::Index Entity::index() const
{
	return _id & INDEX_MASK;
}

/// Returns how many times this slot has been recycled (modulo 256).
Entity::Generation Entity::generation() const
{
	return static_cast<Generation>(_id >> INDEX_BITS);
}

/// Tells whether this is the null entity.
bool Entity::isNull() const
{
	return _id == NULL_ID;
}

}
