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

#include "MosaicIdGenerator.h"
#include "catapult/crypto/Hashes.h"

namespace catapult { namespace model {

	MosaicId GenerateMosaicId(const Address& owner, MosaicNonce nonce) noexcept {
		Hash256 result;
		crypto::Sha3_256_Builder sha3;
		sha3.update({
			{ reinterpret_cast<const uint8_t*>(&nonce), sizeof(MosaicNonce) },
			owner
		});
		sha3.final(result);

		// clear high bit
		constexpr uint64_t Namespace_Flag = 1ull << 63;
		return MosaicId(reinterpret_cast<const uint64_t&>(result[0]) & ~Namespace_Flag);
	}
}}
