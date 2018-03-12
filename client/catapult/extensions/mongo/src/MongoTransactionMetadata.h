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

		/// Creates a new metadata around \a element, \a height and \a index.
		explicit MongoTransactionMetadata(const model::TransactionElement& element, Height height, uint32_t index)
				: MongoTransactionMetadata(
						element.EntityHash,
						element.MerkleComponentHash,
						*element.OptionalExtractedAddresses,
						height,
						index)
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

	private:
		MongoTransactionMetadata(
				const Hash256& entityHash,
				const Hash256& merkleComponentHash,
				const model::AddressSet& addresses,
				Height height,
				uint32_t index)
				: EntityHash(entityHash)
				, MerkleComponentHash(merkleComponentHash)
				, Addresses(addresses)
				, Height(height)
				, Index(index)
		{}

	public:
		/// The object id.
		/// \note bsoncxx::oid constructor is calling bson_oid_init with \c nullptr context,
		///       which uses default threadsafe context from bson_context_get_default to produce process-unique identifiers.
		bsoncxx::oid ObjectId;

		/// The transaction hash.
		const Hash256& EntityHash;

		/// The modified hash that should be used as a hash in the merkle tree.
		const Hash256& MerkleComponentHash;

		/// The addresses involved in the transaction.
		const model::AddressSet& Addresses;

		/// The height.
		catapult::Height Height;

		/// The index of the transaction in the containing block.
		uint32_t Index;
	};
}}
