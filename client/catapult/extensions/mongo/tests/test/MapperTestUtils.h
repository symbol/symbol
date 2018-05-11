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

#pragma once
#include "mongo/src/mappers/MapperInclude.h"
#include "catapult/utils/Casting.h"
#include "tests/test/nodeps/Conversions.h"
#include <cstring>
#include <vector>

namespace catapult {
	namespace mocks { struct MockTransaction; }
	namespace model {
		struct Block;
		struct Cosignature;
		struct EmbeddedTransaction;
		struct Transaction;
		struct VerifiableEntity;
	}
	namespace mongo { struct MongoTransactionMetadata; }
	namespace state { struct AccountState; }
}

namespace catapult { namespace test {

	// region accessors

	/// Returns binary data \a name from a document (\a doc).
	template<typename TDocument>
	auto GetBinary(const TDocument& doc, const std::string& name) {
		return doc[name].get_binary().bytes;
	}

	/// Returns binary data \a name from a document (\a doc) as an array.
	template<size_t N, typename TDocument>
	std::array<uint8_t, N> GetBinaryArray(const TDocument& doc, const std::string& name) {
		std::array<uint8_t, N> value;
		std::memcpy(value.data(), GetBinary(doc, name), value.size());
		return value;
	}

	/// Returns value \a name from a document (\a doc) as an unsigned 8 bit value.
	template<typename TDocument>
	auto GetUint8(const TDocument& doc, const std::string& name) {
		return utils::checked_cast<int32_t, uint8_t>(doc[name].get_int32().value);
	}

	/// Returns value \a name from a document (\a doc) as an unsigned 32 bit value.
	template<typename TDocument>
	auto GetUint32(const TDocument& doc, const std::string& name) {
		return static_cast<uint32_t>(doc[name].get_int32().value);
	}

	/// Returns value \a name from a document (\a doc) as an unsigned 64 bit value.
	template<typename TDocument>
	auto GetUint64(const TDocument& doc, const std::string& name) {
		return static_cast<uint64_t>(doc[name].get_int64().value);
	}

	/// Returns number of fields in a document \a view.
	template<typename TDocumentView>
	size_t GetFieldCount(const TDocumentView& view) {
		return static_cast<size_t>(std::distance(view.cbegin(), view.cend()));
	}

	/// Converts binary field \a name from a document (\a doc) to a hash.
	template<typename TDocument>
	Hash256 GetHashValue(const TDocument& doc, const std::string& name) {
		return GetBinaryArray<Hash256_Size>(doc, name);
	}

	/// Converts binary field \a name from a document (\a doc) to a 512 bit hash.
	template<typename TDocument>
	Hash512 GetHash512Value(const TDocument& doc, const std::string& name) {
		return GetBinaryArray<Hash512_Size>(doc, name);
	}

	/// Converts binary field \a name from a document (\a doc) to a public key.
	template<typename TDocument>
	Key GetKeyValue(const TDocument& doc, const std::string& name) {
		return GetBinaryArray<Key_Size>(doc, name);
	}

	/// Converts binary field \a name from a document (\a doc) to a signature.
	template<typename TDocument>
	Signature GetSignatureValue(const TDocument& doc, const std::string& name) {
		return GetBinaryArray<Signature_Size>(doc, name);
	}

	/// Converts binary field \a name from a document (\a doc) to a (decoded) address.
	template<typename TDocument>
	Address GetAddressValue(const TDocument& doc, const std::string& name) {
		return GetBinaryArray<Address_Decoded_Size>(doc, name);
	}

	// endregion

	// region asserts

	/// Verifies that model \a transaction is equal to db entity (\a dbTransaction).
	void AssertEqualEmbeddedTransactionData(const model::EmbeddedTransaction& transaction, const bsoncxx::document::view& dbTransaction);

	/// Verifies that model \a entity is equal to db entity (\a dbEntity).
	void AssertEqualVerifiableEntityData(const model::VerifiableEntity& entity, const bsoncxx::document::view& dbEntity);

	/// Verifies that model \a transaction is equal to db transaction (\a dbTransaction).
	void AssertEqualTransactionData(const model::Transaction& transaction, const bsoncxx::document::view& dbTransaction);

	/// Verifies that \a metadata matches transaction metadata (\a dbTransactionMetadata) in db.
	void AssertEqualTransactionMetadata(
			const mongo::MongoTransactionMetadata& metadata,
			const bsoncxx::document::view& dbTransactionMetadata);

	/// Verifies that model \a block is equal to db block (\a dbBlock).
	void AssertEqualBlockData(const model::Block& block, const bsoncxx::document::view& dbBlock);

	/// Verifies that \a hash, \a generationHash, \a totalFee, \a numTransactions and \a merkleTree match
	/// block metadata (\a dbBlockMetadata) in db.
	void AssertEqualBlockMetadata(
			const Hash256& hash,
			const Hash256& generationHash,
			Amount totalFee,
			int32_t numTransactions,
			const std::vector<Hash256>& merkleTree,
			const bsoncxx::document::view& dbBlockMetadata);

	/// Verifies that db account (\a dbAccount) and model \a accountState are equivalent.
	void AssertEqualAccountState(const state::AccountState& accountState, const bsoncxx::document::view& dbAccount);

	/// Verifies that mock \a transaction and db transaction (\a dbTransaction) are equivalent.
	void AssertEqualMockTransactionData(const mocks::MockTransaction& transaction, const bsoncxx::document::view& dbTransaction);

	/// Verifies that db cosignatures (\a dbCosignatures) and model \a expectedCosignatures are equivalent.
	void AssertEqualCosignatures(const std::vector<model::Cosignature>& expectedCosignatures, const bsoncxx::array::view& dbCosignatures);

	// endregion
}}
