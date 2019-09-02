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
#include "EntityType.h"
#include "NetworkInfo.h"
#include "catapult/utils/Casting.h"
#include "catapult/types.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for an entity body.
	template<typename THeader>
	struct EntityBody : public THeader {
	public:
		/// Entity signer's public key.
		Key SignerPublicKey;

		/// Entity version.
		uint16_t Version;

		/// Entity type.
		EntityType Type;

		/// Returns network of an entity, as defined in NetworkInfoTraits.
		NetworkIdentifier Network() const {
			return static_cast<NetworkIdentifier>(Version >> 8);
		}

		/// Returns version of an entity.
		uint8_t EntityVersion() const {
			return static_cast<uint8_t>(Version & 0xFF);
		}
	};

#pragma pack(pop)

	/// Creates a version field out of given entity \a version and \a networkIdentifier.
	constexpr uint16_t MakeVersion(NetworkIdentifier networkIdentifier, uint8_t version) noexcept {
		return static_cast<uint16_t>(utils::to_underlying_type(networkIdentifier) << 8 | version);
	}
}}
