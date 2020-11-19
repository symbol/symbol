/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "PacketPayloadBuilder.h"

namespace catapult { namespace ionet {

	/// Factory for creating common packet payloads.
	class PacketPayloadFactory {
	public:
		/// Creates a packet payload with the specified packet \a type around a single entity (\a pEntity).
		template<typename TEntity>
		static PacketPayload FromEntity(PacketType type, const std::shared_ptr<TEntity>& pEntity) {
			return FromEntities<TEntity>(type, { pEntity });
		}

		/// Creates a packet payload with the specified packet \a type around multiple \a entities.
		template<typename TEntity>
		static PacketPayload FromEntities(PacketType type, const std::vector<std::shared_ptr<TEntity>>& entities) {
			PacketPayloadBuilder builder(type);
			builder.appendEntities(entities);
			return builder.build();
		}

		/// Creates a packet payload with the specified packet \a type around a fixed size structure \a range.
		template<typename TStructure>
		static PacketPayload FromFixedSizeRange(PacketType type, model::EntityRange<TStructure>&& range) {
			PacketPayloadBuilder builder(type);
			builder.appendRange(std::move(range));
			return builder.build();
		}
	};
}}
