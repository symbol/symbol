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
#include "EntityType.h"
#include "NetworkIdentifier.h"
#include "catapult/types.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for an entity body.
	template<typename THeader>
	struct EntityBody : public THeader {
	public:
		/// Entity signer's public key.
		Key SignerPublicKey;

		/// Reserved padding to align end of EntityBody on 8-byte boundary.
		uint32_t EntityBody_Reserved1;

		/// Entity version.
		uint8_t Version;

		/// Entity network.
		NetworkIdentifier Network;

		/// Entity type.
		EntityType Type;
	};

#pragma pack(pop)
}}
