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
#include "catapult/model/ContainerTypes.h"
#include "catapult/model/Elements.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/types.h"
#include <bsoncxx/oid.hpp>

namespace catapult { namespace mongo {

	/// Mongo transaction metadata.
	struct MongoTransactionMetadata {
	public:
		/// Creates a new metadata around \a element.
		explicit MongoTransactionMetadata(const model::TransactionElement& element)
				: MongoTransactionMetadata(element, catapult::Height(), 0)
		{}

		/// Creates a new metadata around \a info.
		explicit MongoTransactionMetadata(const model::TransactionInfo& transactionInfo)
				: MongoTransactionMetadata(
						transactionInfo.EntityHash,
						transactionInfo.MerkleComponentHash,
						*transactionInfo.OptionalExtractedAddresses,
						catapult::Height(),
						0)
		{}

		/// Creates a new metadata around \a element, \a height and \a index.
		MongoTransactionMetadata(const model::TransactionElement& element, Height height, uint32_t index)
				: MongoTransactionMetadata(
						element.EntityHash,
						element.MerkleComponentHash,
						*element.OptionalExtractedAddresses,
						height,
						index)
		{}

	private:
		MongoTransactionMetadata(
				const Hash256& entityHash,
				const Hash256& merkleComponentHash,
				const model::UnresolvedAddressSet& addresses,
				Height height,
				uint32_t index)
				: EntityHash(entityHash)
				, MerkleComponentHash(merkleComponentHash)
				, Addresses(addresses)
				, Height(height)
				, Index(index)
		{}

	public:
		/// Object id.
		/// \note bsoncxx::oid constructor is calling bson_oid_init with \c nullptr context,
		///       which uses default threadsafe context from bson_context_get_default to produce process-unique identifiers.
		bsoncxx::oid ObjectId;

		/// Transaction hash.
		const Hash256& EntityHash;

		/// Modified hash that should be used as a hash in the merkle tree.
		const Hash256& MerkleComponentHash;

		/// Addresses involved in the transaction.
		const model::UnresolvedAddressSet& Addresses;

		/// Height.
		catapult::Height Height;

		/// Index of the transaction in the containing block.
		uint32_t Index;
	};
}}
