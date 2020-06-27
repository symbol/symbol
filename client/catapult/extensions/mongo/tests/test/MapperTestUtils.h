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
#include "sdk/src/extensions/ConversionExtensions.h"
#include "mongo/src/mappers/MapperInclude.h"
#include "catapult/utils/Casting.h"
#include "tests/test/nodeps/Conversions.h"
#include <cstring>
#include <vector>

namespace catapult {
	namespace mocks { struct MockTransaction; }
	namespace model {
		struct Block;
		struct BlockElement;
		struct Cosignature;
		struct EmbeddedTransaction;
		struct Receipt;
		struct Transaction;
		struct VerifiableEntity;
	}
	namespace mongo { struct MongoTransactionMetadata; }
	namespace state { struct AccountState; }
}

namespace catapult { namespace test {

	// region accessors

	/// Gets the binary data \a name from a document (\a doc).
	template<typename TDocument>
	auto GetBinary(const TDocument& doc, const std::string& name) {
		return doc[name].get_binary().bytes;
	}

	/// Gets the binary data \a name from a document (\a doc) as an array.
	template<size_t N, typename TDocument>
	std::array<uint8_t, N> GetBinaryArray(const TDocument& doc, const std::string& name) {
		std::array<uint8_t, N> value;
		std::memcpy(value.data(), GetBinary(doc, name), value.size());
		return value;
	}

	/// Gets the value \a name from a document (\a doc) as an signed 8 bit value.
	template<typename TDocument>
	auto GetInt8(const TDocument& doc, const std::string& name) {
		return utils::checked_cast<int32_t, int8_t>(doc[name].get_int32().value);
	}

	/// Gets the value \a name from a document (\a doc) as an signed 32 bit value.
	template<typename TDocument>
	auto GetInt32(const TDocument& doc, const std::string& name) {
		return doc[name].get_int32().value;
	}

	/// Gets the value \a name from a document (\a doc) as an signed 64 bit value.
	template<typename TDocument>
	auto GetInt64(const TDocument& doc, const std::string& name) {
		return doc[name].get_int64().value;
	}

	/// Gets the value \a name from a document (\a doc) as an unsigned 8 bit value.
	template<typename TDocument>
	auto GetUint8(const TDocument& doc, const std::string& name) {
		return utils::checked_cast<int32_t, uint8_t>(doc[name].get_int32().value);
	}

	/// Gets the value \a name from a document (\a doc) as an unsigned 32 bit value.
	template<typename TDocument>
	auto GetUint32(const TDocument& doc, const std::string& name) {
		return static_cast<uint32_t>(doc[name].get_int32().value);
	}

	/// Gets the value \a name from a document (\a doc) as an unsigned 64 bit value.
	template<typename TDocument>
	auto GetUint64(const TDocument& doc, const std::string& name) {
		return static_cast<uint64_t>(doc[name].get_int64().value);
	}

	/// Gets the number of fields in a document \a view.
	template<typename TDocumentView>
	size_t GetFieldCount(const TDocumentView& view) {
		return static_cast<size_t>(std::distance(view.cbegin(), view.cend()));
	}

	/// Converts binary field \a name from a document (\a doc) to a hash.
	template<typename TDocument>
	Hash256 GetHashValue(const TDocument& doc, const std::string& name) {
		return GetBinaryArray<Hash256::Size>(doc, name);
	}

	/// Converts binary field \a name from a document (\a doc) to a generation hash.
	template<typename TDocument>
	GenerationHash GetGenerationHashValue(const TDocument& doc, const std::string& name) {
		return GetBinaryArray<Hash256::Size>(doc, name);
	}

	/// Converts binary field \a name from a document (\a doc) to a 512 bit hash.
	template<typename TDocument>
	Hash512 GetHash512Value(const TDocument& doc, const std::string& name) {
		return GetBinaryArray<Hash512::Size>(doc, name);
	}

	/// Converts binary field \a name from a document (\a doc) to a public key.
	template<typename TDocument>
	Key GetKeyValue(const TDocument& doc, const std::string& name) {
		return GetBinaryArray<Key::Size>(doc, name);
	}

	/// Converts binary field \a name from a document (\a doc) to a public voting key.
	template<typename TDocument>
	VotingKey GetVotingKeyValue(const TDocument& doc, const std::string& name) {
		return GetBinaryArray<VotingKey::Size>(doc, name);
	}

	/// Converts binary field \a name from a document (\a doc) to a signature.
	template<typename TDocument>
	Signature GetSignatureValue(const TDocument& doc, const std::string& name) {
		return GetBinaryArray<Signature::Size>(doc, name);
	}

	/// Converts binary field \a name from a document (\a doc) to a (decoded) address.
	template<typename TDocument>
	Address GetAddressValue(const TDocument& doc, const std::string& name) {
		return GetBinaryArray<Address::Size>(doc, name);
	}

	/// Converts binary field \a name from a document (\a doc) to a (decoded) address.
	template<typename TDocument>
	UnresolvedAddress GetUnresolvedAddressValue(const TDocument& doc, const std::string& name) {
		return extensions::CopyToUnresolvedAddress(GetAddressValue(doc, name));
	}

	/// Converts binary \a source to a byte array.
	template<typename TByteArray, typename TMongoContainer>
	TByteArray GetByteArrayFromMongoSource(const TMongoContainer& source) {
		TByteArray byteArray;
		std::memcpy(byteArray.data(), source.get_binary().bytes, byteArray.size());
		return byteArray;
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

	/// Verifies that \a blockElement, \a totalFee, \a numTransactions, \a numStatements,
	/// \a transactionMerkleTree and \a statementMerkleTree match block metadata (\a dbBlockMetadata) in db.
	void AssertEqualBlockMetadata(
			const model::BlockElement& blockElement,
			Amount totalFee,
			int32_t numTransactions,
			int32_t numStatements,
			const std::vector<Hash256>& transactionMerkleTree,
			const std::vector<Hash256>& statementMerkleTree,
			const bsoncxx::document::view& dbBlockMetadata);

	/// Verifies that model \a accountState and db account (\a dbAccount) are equivalent.
	void AssertEqualAccountState(const state::AccountState& accountState, const bsoncxx::document::view& dbAccount);

	/// Verifies that mock \a transaction and db transaction (\a dbTransaction) are equivalent.
	void AssertEqualMockTransactionData(const mocks::MockTransaction& transaction, const bsoncxx::document::view& dbTransaction);

	/// Verifies that model \a expectedCosignatures and db cosignatures (\a dbCosignatures) are equivalent.
	void AssertEqualCosignatures(const std::vector<model::Cosignature>& expectedCosignatures, const bsoncxx::array::view& dbCosignatures);

	/// Verifies that model \a receipt and db receipt (\a dbReceipt) are equivalent.
	void AssertEqualReceiptData(const model::Receipt& receipt, const bsoncxx::document::view& dbReceipt);

	// endregion
}}
