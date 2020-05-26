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

#include "MetadataKey.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/model/ResolverContext.h"
#include "catapult/utils/Casting.h"
#include "catapult/exceptions.h"

namespace catapult { namespace state {

	MetadataKey::MetadataKey(const model::PartialMetadataKey& partialKey)
			: m_partialKey(partialKey)
			, m_targetId(0)
			, m_metadataType(model::MetadataType::Account)
			, m_uniqueKey(generateUniqueKey())
	{}

	MetadataKey::MetadataKey(const model::PartialMetadataKey& partialKey, MosaicId mosaicId)
			: m_partialKey(partialKey)
			, m_targetId(mosaicId.unwrap())
			, m_metadataType(model::MetadataType::Mosaic)
			, m_uniqueKey(generateUniqueKey())
	{}

	MetadataKey::MetadataKey(const model::PartialMetadataKey& partialKey, NamespaceId namespaceId)
			: m_partialKey(partialKey)
			, m_targetId(namespaceId.unwrap())
			, m_metadataType(model::MetadataType::Namespace)
			, m_uniqueKey(generateUniqueKey())
	{}

	const Hash256& MetadataKey::uniqueKey() const {
		return m_uniqueKey;
	}

	const Address& MetadataKey::sourceAddress() const {
		return m_partialKey.SourceAddress;
	}

	const Address& MetadataKey::targetAddress() const {
		return m_partialKey.TargetAddress;
	}

	uint64_t MetadataKey::scopedMetadataKey() const {
		return m_partialKey.ScopedMetadataKey;
	}

	uint64_t MetadataKey::targetId() const {
		return m_targetId;
	}

	model::MetadataType MetadataKey::metadataType() const {
		return m_metadataType;
	}

	MosaicId MetadataKey::mosaicTarget() const {
		require(model::MetadataType::Mosaic, "mosaicTarget");
		return MosaicId(m_targetId);
	}

	NamespaceId MetadataKey::namespaceTarget() const {
		require(model::MetadataType::Namespace, "namespaceTarget");
		return NamespaceId(m_targetId);
	}

	void MetadataKey::require(model::MetadataType metadataType, const char* name) const {
		if (m_metadataType == metadataType)
			return;

		std::ostringstream out;
		out
				<< "function \"" << name << "\" requires metadata type " << static_cast<uint16_t>(metadataType)
				<< " but was " << static_cast<uint16_t>(m_metadataType);
		CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
	}

	Hash256 MetadataKey::generateUniqueKey() const {
		crypto::Sha3_256_Builder builder;

		for (const auto* pKey : { &m_partialKey.SourceAddress, &m_partialKey.TargetAddress })
			builder.update(*pKey);

		for (const auto value : { m_partialKey.ScopedMetadataKey, m_targetId })
			builder.update({ reinterpret_cast<const uint8_t*>(&value), sizeof(uint64_t) });

		builder.update({ reinterpret_cast<const uint8_t*>(&m_metadataType), sizeof(model::MetadataType) });

		Hash256 uniqueKey;
		builder.final(uniqueKey);
		return uniqueKey;
	}

	namespace {
		MetadataKey ResolveMetadataKey(
				const model::PartialMetadataKey& partialKey,
				const model::MetadataTarget& target,
				const std::function<MosaicId (uint64_t)>& idToMosaicIdResolver) {
			switch (target.Type) {
			case model::MetadataType::Account:
				return MetadataKey(partialKey);

			case model::MetadataType::Mosaic:
				return MetadataKey(partialKey, idToMosaicIdResolver(target.Id));

			case model::MetadataType::Namespace:
				return MetadataKey(partialKey, NamespaceId(target.Id));
			}

			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot resolve metadata key with unsupported type", static_cast<uint16_t>(target.Type));
		}
	}

	MetadataKey CreateMetadataKey(const model::PartialMetadataKey& partialKey, const model::MetadataTarget& target) {
		return ResolveMetadataKey(partialKey, target, [](auto id) {
			return MosaicId(id);
		});
	}

	MetadataKey ResolveMetadataKey(
			const model::UnresolvedPartialMetadataKey& partialKey,
			const model::MetadataTarget& target,
			const model::ResolverContext& resolvers) {
		auto resolvedPartialKey = model::PartialMetadataKey{
			partialKey.SourceAddress,
			resolvers.resolve(partialKey.TargetAddress),
			partialKey.ScopedMetadataKey
		};
		return ResolveMetadataKey(resolvedPartialKey, target, [&resolvers](auto id) {
			return resolvers.resolve(UnresolvedMosaicId(id));
		});
	}
}}
