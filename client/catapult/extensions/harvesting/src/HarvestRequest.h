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
#include "catapult/types.h"

namespace catapult { namespace harvesting {

	struct HarvestRequestIdentifier_tag { static constexpr size_t Size = 32; };
	using HarvestRequestIdentifier = utils::ByteArray<HarvestRequestIdentifier_tag>;

	/// Harvest request operation.
	enum class HarvestRequestOperation : uint8_t {
		/// Add unlocked harvester.
		Add,

		/// Remove unlocked harvester.
		Remove
	};

	/// Harvest request.
	struct HarvestRequest {
	public:
		/// Request operation.
		HarvestRequestOperation Operation;

		/// Public key of the main account that initiated the request.
		Key MainAccountPublicKey;

		/// Encrypted payload.
		/// \note This decrypts into BlockGeneratorAccountDescriptor.
		RawBuffer EncryptedPayload;

	public:
		/// Gets the decrypted payload size.
		static size_t DecryptedPayloadSize();

		/// Gets the encrypted payload size.
		static size_t EncryptedPayloadSize();
	};

	/// Gets a unique identifier for \a request.
	HarvestRequestIdentifier GetRequestIdentifier(const HarvestRequest& request);
}}
