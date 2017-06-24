#pragma once
#include "catapult/types.h"
#include <bsoncxx/oid.hpp>

namespace catapult { namespace mongo { namespace plugins {

	/// Mongo transaction metadata.
	struct MongoTransactionMetadata {
	public:
		/// Creates a new metadata around \a entityHash and \a merkleComponentHash.
		explicit MongoTransactionMetadata(const Hash256& entityHash, const Hash256& merkleComponentHash)
				: MongoTransactionMetadata(entityHash, merkleComponentHash, catapult::Height(), 0)
		{}

		/// Creates a new metadata around \a entityHash, \a merkleComponentHash, \a height and \a index.
		explicit MongoTransactionMetadata(const Hash256& entityHash, const Hash256& merkleComponentHash, Height height, uint32_t index)
				: EntityHash(entityHash)
				, MerkleComponentHash(merkleComponentHash)
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

		/// The height.
		catapult::Height Height;

		/// The index of the transaction in the containing block.
		uint32_t Index;
	};
}}}
