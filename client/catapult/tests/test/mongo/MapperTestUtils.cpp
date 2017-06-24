#include "MapperTestUtils.h"
#include "plugins/mongo/coremongo/src/MongoTransactionMetadata.h"
#include "catapult/model/Block.h"
#include "catapult/state/AccountState.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"
#include <bsoncxx/types.hpp>

namespace catapult { namespace test {

	namespace {
		template<typename TEntity>
		void AssertEqualEntityData(const TEntity& entity, const bsoncxx::document::view& dbEntity) {
			EXPECT_EQ(test::ToHexString(entity.Signer), test::ToHexString(GetBinary(dbEntity, "signer"), Key_Size));

			EXPECT_EQ(entity.Version, dbEntity["version"].get_int32().value);
			EXPECT_EQ(utils::to_underlying_type(entity.Type), dbEntity["type"].get_int32().value);
		}
	}

	void AssertEqualEmbeddedEntityData(const model::EmbeddedEntity& entity, const bsoncxx::document::view& dbEntity) {
		AssertEqualEntityData(entity, dbEntity);
	}

	void AssertEqualVerifiableEntityData(const model::VerifiableEntity& entity, const bsoncxx::document::view& dbEntity) {
		EXPECT_EQ(test::ToHexString(entity.Signature), test::ToHexString(GetBinary(dbEntity, "signature"), Signature_Size));
		AssertEqualEntityData(entity, dbEntity);
	}

	void AssertEqualTransactionData(const model::Transaction& transaction, const bsoncxx::document::view& dbTransaction) {
		AssertEqualVerifiableEntityData(transaction, dbTransaction);
		EXPECT_EQ(transaction.Fee.unwrap(), GetUint64(dbTransaction, "fee"));
		EXPECT_EQ(transaction.Deadline.unwrap(), GetUint64(dbTransaction, "deadline"));
	}

	namespace {
		template<typename TDocument>
		auto GetHashHexString(const TDocument& doc, const std::string& name) {
			return test::ToHexString(GetBinary(doc, name), Hash256_Size);
		}
	}

	void AssertEqualTransactionMetadata(
			const mongo::plugins::MongoTransactionMetadata& metadata,
			const bsoncxx::document::view& dbTransactionMetadata) {
		EXPECT_EQ(test::ToHexString(metadata.EntityHash), GetHashHexString(dbTransactionMetadata, "hash"));
		EXPECT_EQ(test::ToHexString(metadata.MerkleComponentHash), GetHashHexString(dbTransactionMetadata, "merkleComponentHash"));

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
		EXPECT_EQ(test::ToHexString(block.PreviousBlockHash), GetHashHexString(dbBlock, "previousBlockHash"));
		EXPECT_EQ(test::ToHexString(block.BlockTransactionsHash), GetHashHexString(dbBlock, "blockTransactionsHash"));
	}

	void AssertEqualBlockMetadata(
			const Hash256& hash,
			const Hash256& generationHash,
			Amount totalFee,
			int32_t numTransactions,
			const bsoncxx::document::view& dbBlockMetadata) {
		EXPECT_EQ(4u, GetFieldCount(dbBlockMetadata));
		EXPECT_EQ(test::ToHexString(hash), GetHashHexString(dbBlockMetadata, "hash"));
		EXPECT_EQ(test::ToHexString(generationHash), GetHashHexString(dbBlockMetadata, "generationHash"));
		EXPECT_EQ(totalFee.unwrap(), GetUint64(dbBlockMetadata, "totalFee"));
		EXPECT_EQ(numTransactions, dbBlockMetadata["numTransactions"].get_int32().value);
	}

	void AssertEqualAccountState(const state::AccountState& accountState, const bsoncxx::document::view& dbAccount) {
		EXPECT_EQ(test::ToHexString(accountState.Address), test::ToHexString(test::GetBinary(dbAccount, "address"), Address_Decoded_Size));
		EXPECT_EQ(accountState.AddressHeight.unwrap(), test::GetUint64(dbAccount, "addressHeight"));
		EXPECT_EQ(
			test::ToHexString(Height(0) != accountState.PublicKeyHeight ? accountState.PublicKey : Key{}),
			test::ToHexString(test::GetBinary(dbAccount, "publicKey"), Key_Size));
		EXPECT_EQ(accountState.PublicKeyHeight.unwrap(), test::GetUint64(dbAccount, "publicKeyHeight"));

		auto dbImportances = dbAccount["importances"].get_array().value;
		const auto& accountImportances = accountState.ImportanceInfo;
		size_t numImportances = 0;
		for (const auto& importanceElement : dbImportances) {
			auto importanceDocument = importanceElement.get_document();
			auto importanceHeight = test::GetUint64(importanceDocument.view(), "height");

			auto importance = accountImportances.get(model::ImportanceHeight(importanceHeight));
			EXPECT_EQ(importance.unwrap(), test::GetUint64(importanceDocument.view(), "value"));
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
			auto id = MosaicId(test::GetUint64(mosaicDocument.view(), "id"));
			EXPECT_EQ(accountState.Balances.get(id).unwrap(), test::GetUint64(mosaicDocument.view(), "amount"));
			++numMosaics;
		}

		EXPECT_EQ(accountState.Balances.size(), numMosaics);
	}

	void AssertEqualMockTransactionData(const mocks::MockTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
		AssertEqualTransactionData(transaction, dbTransaction);
		EXPECT_EQ(test::ToHexString(transaction.Recipient), test::ToHexString(GetBinary(dbTransaction, "recipient"), Key_Size));
		EXPECT_EQ(
				test::ToHexString(transaction.DataPtr(), transaction.Data.Size),
				test::ToHexString(GetBinary(dbTransaction, "data"), transaction.Data.Size));
	}
}}
