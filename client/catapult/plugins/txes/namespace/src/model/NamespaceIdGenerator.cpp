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

#include "NamespaceIdGenerator.h"
#include "NamespaceConstants.h"
#include "catapult/crypto/Hashes.h"

namespace catapult { namespace model {

	NamespaceId GenerateRootNamespaceId(const RawString& name) noexcept {
		return GenerateNamespaceId(Namespace_Base_Id, name);
	}

	NamespaceId GenerateNamespaceId(NamespaceId parentId, const RawString& name) noexcept {
		Hash256 result;
		crypto::Sha3_256_Builder sha3;
		sha3.update({
			{ reinterpret_cast<const uint8_t*>(&parentId), sizeof(NamespaceId) },
			{ reinterpret_cast<const uint8_t*>(name.pData), name.Size }
		});
		sha3.final(result);

		// set high bit
		constexpr uint64_t Namespace_Flag = 1ull << 63;
		return NamespaceId(reinterpret_cast<const uint64_t&>(result[0]) | Namespace_Flag);
	}
}}
