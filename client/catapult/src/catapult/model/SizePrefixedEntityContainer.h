/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "ContiguousEntityContainer.h"
#include "SizePrefixedEntity.h"

namespace catapult { namespace model {

/// Binary layout for a container of size prefixed entities.
/// \a CLASS_PREFIX is the prefix of the container class name.
/// \a PROPERTY_NAME is the name of the exposed property.
#define DEFINE_SIZE_PREFIXED_ENTITY_CONTAINER(CLASS_PREFIX, PROPERTY_NAME) \
	template<typename TEntityHeader, typename TComponentEntity> \
	struct CLASS_PREFIX##Container : public TEntityHeader { \
		/* Gets a container wrapping the entities contained in this container with the desired error policy (\a errorPolicy). */ \
		auto PROPERTY_NAME(EntityContainerErrorPolicy errorPolicy = EntityContainerErrorPolicy::Throw) { \
			return MakeContiguousEntityContainer(PROPERTY_NAME##Ptr(), Get##CLASS_PREFIX##PayloadSize(*this), errorPolicy); \
		} \
		\
		/* Gets a container wrapping the const entities contained in this container with the desired error policy (\a errorPolicy). */ \
		auto PROPERTY_NAME(EntityContainerErrorPolicy errorPolicy = EntityContainerErrorPolicy::Throw) const { \
			return MakeContiguousEntityContainer(PROPERTY_NAME##Ptr(), Get##CLASS_PREFIX##PayloadSize(*this), errorPolicy); \
		} \
		\
		/* Gets a pointer to entities contained in this container. */ \
		TComponentEntity* PROPERTY_NAME##Ptr() { \
			return TEntityHeader::Size <= sizeof(TEntityHeader) || 0 == Get##CLASS_PREFIX##PayloadSize(*this) \
					? nullptr \
					: reinterpret_cast<TComponentEntity*>(reinterpret_cast<uint8_t*>(this) + sizeof(TEntityHeader)); \
		} \
		\
		/* Gets a const pointer to entities contained in this container. */ \
		const TComponentEntity* PROPERTY_NAME##Ptr() const { \
			return TEntityHeader::Size <= sizeof(TEntityHeader) || 0 == Get##CLASS_PREFIX##PayloadSize(*this) \
					? nullptr \
					: reinterpret_cast<const TComponentEntity*>(reinterpret_cast<const uint8_t*>(this) + sizeof(TEntityHeader)); \
		} \
	};

#pragma pack(push, 1)

DEFINE_SIZE_PREFIXED_ENTITY_CONTAINER(Transaction, Transactions)

#pragma pack(pop)
}}
