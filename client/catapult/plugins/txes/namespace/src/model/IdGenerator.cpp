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

#include "IdGenerator.h"
#include "NamespaceConstants.h"
#include "catapult/crypto/Hashes.h"

namespace catapult { namespace model {

	namespace {
		template<typename TId>
		TId GenerateId(uint64_t parentId, const RawString& name) noexcept {
			Hash256 result;
			crypto::Sha3_256_Builder sha3;
			sha3.update({
				{ reinterpret_cast<const uint8_t*>(&parentId), sizeof(NamespaceId) },
				{ reinterpret_cast<const uint8_t*>(name.pData), name.Size } });
			sha3.final(result);
			return reinterpret_cast<const TId&>(*result.data());
		}
	}

	NamespaceId GenerateRootNamespaceId(const RawString& name) noexcept {
		return GenerateNamespaceId(Namespace_Base_Id, name);
	}

	NamespaceId GenerateNamespaceId(NamespaceId parentId, const RawString& name) noexcept {
		return GenerateId<NamespaceId>(parentId.unwrap(), name);
	}

	MosaicId GenerateMosaicId(NamespaceId namespaceId, const RawString& name) noexcept {
		return GenerateId<MosaicId>(namespaceId.unwrap(), name);
	}
}}
