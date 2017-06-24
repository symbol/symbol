#pragma once
#include "catapult/types.h"

namespace bsoncxx { inline namespace v_noabi {
	namespace document { class view; }
	namespace types { struct b_binary; }
}}

namespace catapult {
	namespace model {
		struct Block;
		struct EmbeddedEntity;
		struct Transaction;
		struct VerifiableEntity;
	}
	namespace mocks {
		struct MockTransaction;
	}
	namespace mongo { namespace plugins { struct MongoTransactionMetadata;} }
	namespace state {
		struct AccountState;
	}
}

namespace catapult { namespace test {

	// region accessors

	/// Returns binary data \a name from a document (\a doc).
	template<typename TDocument>
	auto GetBinary(const TDocument& doc, const std::string& name) {
		return doc[name].get_binary().bytes;
	}

	/// Returns value \a name from a document (\a doc) as an unsigned 64 bit value.
	template<typename TDocument>
	auto GetUint64(const TDocument& doc, const std::string& name) {
		return static_cast<uint64_t>(doc[name].get_int64().value);
	}

	/// Returns value \a name from a document (\a doc) as an unsigned 32 bit value.
	template<typename TDocument>
	auto GetUint32(const TDocument& doc, const std::string& name) {
		return static_cast<uint32_t>(doc[name].get_int32().value);
	}

	/// Returns number of fields in a document \a view.
	template<typename TDocumentView>
	size_t GetFieldCount(const TDocumentView& view) {
		return static_cast<size_t>(std::distance(view.cbegin(), view.cend()));
	}

	// endregion

	// region asserts

	/// Verifies that model \a entity is equal to db entity (\a dbEntity).
	void AssertEqualEmbeddedEntityData(const model::EmbeddedEntity& entity, const bsoncxx::document::view& dbEntity);

	/// Verifies that model \a entity is equal to db entity (\a dbEntity).
	void AssertEqualVerifiableEntityData(const model::VerifiableEntity& entity, const bsoncxx::document::view& dbEntity);

	/// Verifies that model \a transaction is equal to db transaction (\a dbTransaction).
	void AssertEqualTransactionData(const model::Transaction& transaction, const bsoncxx::document::view& dbTransaction);

	/// Verifies that \a metadata matches transaction metadata (\a dbTransactionMetadata) in db.
	void AssertEqualTransactionMetadata(
			const mongo::plugins::MongoTransactionMetadata& metadata,
			const bsoncxx::document::view& dbTransactionMetadata);

	/// Verifies that model \a block is equal to db block (\a dbBlock).
	void AssertEqualBlockData(const model::Block& block, const bsoncxx::document::view& dbBlock);

	/// Verifies that \a hash, \a generationHash, \a totalFee and \a numTransactions match
	/// block metadata (\a dbBlockMetadata) in db.
	void AssertEqualBlockMetadata(
			const Hash256& hash,
			const Hash256& generationHash,
			Amount totalFee,
			int32_t numTransactions,
			const bsoncxx::document::view& dbBlockMetadata);

	/// Verifies that db account (\a dbAccount) and model \a accountState are equivalent.
	void AssertEqualAccountState(const state::AccountState& accountState, const bsoncxx::document::view& dbAccount);

	/// Verifies that mock \a transaction and db transaction (\a dbTransaction) are equivalent.
	void AssertEqualMockTransactionData(const mocks::MockTransaction& transaction, const bsoncxx::document::view& dbTransaction);

	// endregion
}}
