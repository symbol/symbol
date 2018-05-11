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

#include "MapperTestUtils.h"
#include "mongo/src/MongoTransactionMetadata.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/model/Block.h"
#include "catapult/model/ContainerTypes.h"
#include "catapult/model/Cosignature.h"
#include "catapult/state/AccountState.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"
#include <bsoncxx/types.hpp>

namespace catapult { namespace test {

	namespace {
		Address ToAddress(const uint8_t* pByteArray) {
			Address address;
			std::memcpy(address.data(), pByteArray, Address_Decoded_Size);
			return address;
		}

		template<typename TEntity>
		void AssertEqualEntityData(const TEntity& entity, const bsoncxx::document::view& dbEntity) {
			EXPECT_EQ(entity.Signer, GetKeyValue(dbEntity, "signer"));

			EXPECT_EQ(entity.Version, dbEntity["version"].get_int32().value);
			EXPECT_EQ(utils::to_underlying_type(entity.Type), dbEntity["type"].get_int32().value);
		}

		void AssertEqualMerkleTree(const std::vector<Hash256>& merkleTree, const bsoncxx::document::view& dbMerkleTree) {
			ASSERT_EQ(merkleTree.size(), std::distance(dbMerkleTree.cbegin(), dbMerkleTree.cend()));

			auto i = 0u;
			for (const auto& dbHash : dbMerkleTree) {
				Hash256 hash;
				mongo::mappers::DbBinaryToModelArray(hash, dbHash.get_binary());
				EXPECT_EQ(merkleTree[i], hash);
				++i;
			}
		}
	}

	void AssertEqualEmbeddedTransactionData(const model::EmbeddedTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
		AssertEqualEntityData(transaction, dbTransaction);
	}

	void AssertEqualVerifiableEntityData(const model::VerifiableEntity& entity, const bsoncxx::document::view& dbEntity) {
		EXPECT_EQ(entity.Signature, GetSignatureValue(dbEntity, "signature"));
		AssertEqualEntityData(entity, dbEntity);
	}

	void AssertEqualTransactionData(const model::Transaction& transaction, const bsoncxx::document::view& dbTransaction) {
		AssertEqualVerifiableEntityData(transaction, dbTransaction);
		EXPECT_EQ(transaction.Fee.unwrap(), GetUint64(dbTransaction, "fee"));
		EXPECT_EQ(transaction.Deadline.unwrap(), GetUint64(dbTransaction, "deadline"));
	}

	void AssertEqualTransactionMetadata(
			const mongo::MongoTransactionMetadata& metadata,
			const bsoncxx::document::view& dbTransactionMetadata) {
		EXPECT_EQ(metadata.EntityHash, GetHashValue(dbTransactionMetadata, "hash"));
		EXPECT_EQ(metadata.MerkleComponentHash, GetHashValue(dbTransactionMetadata, "merkleComponentHash"));
		auto dbAddresses = dbTransactionMetadata["addresses"].get_array().value;
		model::AddressSet addresses;
		for (const auto& dbAddress : dbAddresses) {
			ASSERT_EQ(Address_Decoded_Size, dbAddress.get_binary().size);
			addresses.insert(ToAddress(dbAddress.get_binary().bytes));
		}

		EXPECT_EQ(metadata.Addresses.size(), addresses.size());
		EXPECT_EQ(metadata.Addresses, addresses);
		EXPECT_EQ(metadata.Height, Height(GetUint64(dbTransactionMetadata, "height")));
		EXPECT_EQ(metadata.Index, GetUint32(dbTransactionMetadata, "index"));
	}

	void AssertEqualBlockData(const model::Block& block, const bsoncxx::document::view& dbBlock) {
		// - 4 fields from VerifiableEntity, 5 fields from Block
		EXPECT_EQ(9u, GetFieldCount(dbBlock));
		AssertEqualVerifiableEntityData(block, dbBlock);
		EXPECT_EQ(block.Height.unwrap(), GetUint64(dbBlock, "height"));
		EXPECT_EQ(block.Timestamp.unwrap(), GetUint64(dbBlock, "timestamp"));
		EXPECT_EQ(block.Difficulty.unwrap(), GetUint64(dbBlock, "difficulty"));
		EXPECT_EQ(block.PreviousBlockHash, GetHashValue(dbBlock, "previousBlockHash"));
		EXPECT_EQ(block.BlockTransactionsHash, GetHashValue(dbBlock, "blockTransactionsHash"));
	}

	void AssertEqualBlockMetadata(
			const Hash256& hash,
			const Hash256& generationHash,
			Amount totalFee,
			int32_t numTransactions,
			const std::vector<Hash256>& merkleTree,
			const bsoncxx::document::view& dbBlockMetadata) {
		EXPECT_EQ(5u, GetFieldCount(dbBlockMetadata));
		EXPECT_EQ(hash, GetHashValue(dbBlockMetadata, "hash"));
		EXPECT_EQ(generationHash, GetHashValue(dbBlockMetadata, "generationHash"));
		EXPECT_EQ(totalFee.unwrap(), GetUint64(dbBlockMetadata, "totalFee"));
		EXPECT_EQ(numTransactions, dbBlockMetadata["numTransactions"].get_int32().value);

		auto dbMerkleTree = dbBlockMetadata["merkleTree"].get_array().value;
		AssertEqualMerkleTree(merkleTree, dbMerkleTree);
	}

	void AssertEqualAccountState(const state::AccountState& accountState, const bsoncxx::document::view& dbAccount) {
		EXPECT_EQ(accountState.Address, GetAddressValue(dbAccount, "address"));
		EXPECT_EQ(accountState.AddressHeight.unwrap(), GetUint64(dbAccount, "addressHeight"));
		EXPECT_EQ(Height(0) != accountState.PublicKeyHeight ? accountState.PublicKey : Key{}, GetKeyValue(dbAccount, "publicKey"));
		EXPECT_EQ(accountState.PublicKeyHeight.unwrap(), GetUint64(dbAccount, "publicKeyHeight"));

		auto dbImportances = dbAccount["importances"].get_array().value;
		const auto& accountImportances = accountState.ImportanceInfo;
		size_t numImportances = 0;
		for (const auto& importanceElement : dbImportances) {
			auto importanceDocument = importanceElement.get_document();
			auto importanceHeight = GetUint64(importanceDocument.view(), "height");

			auto importance = accountImportances.get(model::ImportanceHeight(importanceHeight));
			EXPECT_EQ(importance.unwrap(), GetUint64(importanceDocument.view(), "value"));
			++numImportances;
		}

		auto expectedNumImportances = std::count_if(accountImportances.begin(), accountImportances.end(), [](const auto& importanceInfo){
			return model::ImportanceHeight(0) != importanceInfo.Height;
		});
		EXPECT_EQ(expectedNumImportances, numImportances);

		auto dbMosaics = dbAccount["mosaics"].get_array().value;
		size_t numMosaics = 0;
		for (const auto& mosaicElement : dbMosaics) {
			auto mosaicDocument = mosaicElement.get_document();
			auto id = MosaicId(GetUint64(mosaicDocument.view(), "id"));
			EXPECT_EQ(accountState.Balances.get(id).unwrap(), GetUint64(mosaicDocument.view(), "amount"));
			++numMosaics;
		}

		EXPECT_EQ(accountState.Balances.size(), numMosaics);
	}

	void AssertEqualMockTransactionData(const mocks::MockTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
		AssertEqualTransactionData(transaction, dbTransaction);
		EXPECT_EQ(transaction.Recipient, GetKeyValue(dbTransaction, "recipient"));
		EXPECT_EQ(
				ToHexString(transaction.DataPtr(), transaction.Data.Size),
				ToHexString(GetBinary(dbTransaction, "data"), transaction.Data.Size));
	}

	void AssertEqualCosignatures(const std::vector<model::Cosignature>& expectedCosignatures, const bsoncxx::array::view& dbCosignatures) {
		auto iter = dbCosignatures.cbegin();
		for (const auto& expectedCosignature : expectedCosignatures) {
			auto cosignatureView = iter->get_document().view();
			EXPECT_EQ(expectedCosignature.Signer, GetKeyValue(cosignatureView, "signer"));
			EXPECT_EQ(expectedCosignature.Signature, GetSignatureValue(cosignatureView, "signature"));
			++iter;
		}
	}
}}
