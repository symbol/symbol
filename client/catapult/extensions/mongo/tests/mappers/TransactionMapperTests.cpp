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

#include "mongo/src/mappers/TransactionMapper.h"
#include "mongo/src/MongoTransactionPlugin.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/model/Transaction.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace mappers {

#define TEST_CLASS TransactionMapperTests

	namespace {
		// region ArbitraryTransaction

		constexpr model::EntityType Arbitrary_Transaction_Type = static_cast<model::EntityType>(777);

#pragma pack(push, 1)

		struct ArbitraryTransaction : public model::Transaction {
			uint32_t Alpha = 0x12;
			uint32_t Zeta = 0x65;
		};

#pragma pack(pop)

		auto CreateArbitraryTransaction() {
			auto pTransaction = std::make_unique<ArbitraryTransaction>();
			pTransaction->Size = sizeof(ArbitraryTransaction);
			pTransaction->Type = Arbitrary_Transaction_Type;
			test::FillWithRandomData(pTransaction->SignerPublicKey);
			test::FillWithRandomData(pTransaction->Signature);
			return pTransaction;
		}

		// endregion

		bsoncxx::document::value CreateSingleValueDocument(const std::string& key, uint32_t value) {
			bson_stream::document builder;
			builder << key << static_cast<int32_t>(value);
			return builder << bson_stream::finalize;
		}

		enum class DependentDocumentOptions { None, All };

		class MongoArbitraryTransactionPlugin : public MongoTransactionPlugin {
		public:
			explicit MongoArbitraryTransactionPlugin(DependentDocumentOptions dependentDocumentOptions)
					: m_dependentDocumentOptions(dependentDocumentOptions)
			{}

		public:
			model::EntityType type() const override {
				return Arbitrary_Transaction_Type;
			}

			void streamTransaction(bson_stream::document& builder, const model::Transaction& transaction) const override {
				const auto& arbitraryTransaction = static_cast<const ArbitraryTransaction&>(transaction);
				builder
						<< "alpha" << static_cast<int32_t>(arbitraryTransaction.Alpha)
						<< "zeta" << static_cast<int32_t>(arbitraryTransaction.Zeta);
			}

			std::vector<bsoncxx::document::value> extractDependentDocuments(
					const model::Transaction& transaction,
					const MongoTransactionMetadata&) const override {
				if (DependentDocumentOptions::None == m_dependentDocumentOptions)
					return {};

				const auto& arbitraryTransaction = static_cast<const ArbitraryTransaction&>(transaction);
				return {
					CreateSingleValueDocument("sum", arbitraryTransaction.Alpha + arbitraryTransaction.Zeta),
					CreateSingleValueDocument("diff", arbitraryTransaction.Zeta - arbitraryTransaction.Alpha),
					CreateSingleValueDocument("prod", arbitraryTransaction.Alpha * arbitraryTransaction.Zeta)
				};
			}

			bool supportsEmbedding() const override{
				return false;
			}

			const mongo::EmbeddedMongoTransactionPlugin& embeddedPlugin() const override {
				CATAPULT_THROW_RUNTIME_ERROR("embeddedPlugin - not implemented in mock");
			}

		private:
			DependentDocumentOptions m_dependentDocumentOptions;
		};

		void AssertSingleValueDocument(bsoncxx::document::value& dbValue, const std::string& key, uint32_t value) {
			auto view = dbValue.view();
			EXPECT_EQ(1u, test::GetFieldCount(view));
			EXPECT_EQ(value, test::GetUint32(view, key));
		}

		template<typename TAssertAdditionalFieldsFunc>
		void AssertTransaction(
				const bsoncxx::document::value& dbTransaction,
				const model::Transaction& transaction,
				const MongoTransactionMetadata& metadata,
				size_t numExpectedAdditionalFields,
				TAssertAdditionalFieldsFunc assertAdditionalFields) {
			auto view = dbTransaction.view();
			EXPECT_EQ(3u, test::GetFieldCount(view));

			auto oid = view["_id"].get_oid().value;
			EXPECT_EQ(metadata.ObjectId, oid);

			auto metaView = view["meta"].get_document().view();
			EXPECT_EQ(5u, test::GetFieldCount(metaView));
			test::AssertEqualTransactionMetadata(metadata, metaView);

			auto transactionView = view["transaction"].get_document().view();
			EXPECT_EQ(8u + numExpectedAdditionalFields, test::GetFieldCount(transactionView));
			test::AssertEqualTransactionData(transaction, transactionView);
			assertAdditionalFields(transactionView);
		}

		template<typename TAssertAdditionalFieldsFunc>
		void AssertCanMapTransaction(
				const MongoTransactionRegistry& registry,
				size_t numExpectedAdditionalFields,
				TAssertAdditionalFieldsFunc assertAdditionalFields) {
			// Arrange:
			auto pTransaction = CreateArbitraryTransaction();
			model::TransactionElement transactionElement(*pTransaction);
			transactionElement.EntityHash = test::GenerateRandomByteArray<Hash256>();
			transactionElement.MerkleComponentHash = test::GenerateRandomByteArray<Hash256>();
			transactionElement.OptionalExtractedAddresses = test::GenerateRandomUnresolvedAddressSetPointer(3);
			auto metadata = MongoTransactionMetadata(transactionElement, Height(123), 234);

			// Act:
			auto dbModels = ToDbDocuments(*pTransaction, metadata, registry);

			// Assert:
			ASSERT_EQ(1u, dbModels.size());
			AssertTransaction(dbModels[0], *pTransaction, metadata, numExpectedAdditionalFields, assertAdditionalFields);
		}
	}

	TEST(TEST_CLASS, CanMapKnownTransactionType) {
		// Arrange:
		MongoTransactionRegistry registry;
		registry.registerPlugin(std::make_unique<MongoArbitraryTransactionPlugin>(DependentDocumentOptions::None));

		// Assert:
		AssertCanMapTransaction(registry, 2, [](const auto& dbTransaction) {
			EXPECT_EQ(0x12u, test::GetUint32(dbTransaction, "alpha"));
			EXPECT_EQ(0x65u, test::GetUint32(dbTransaction, "zeta"));
		});
	}

	TEST(TEST_CLASS, CanMapUnknownTransactionType) {
		// Arrange:
		MongoTransactionRegistry registry;

		// Assert:
		AssertCanMapTransaction(registry, 1, [](const auto& dbTransaction) {
			auto expectedBinHeader = test::HexStringToVector("1200000065000000");
			EXPECT_EQ_MEMORY(&expectedBinHeader[0], test::GetBinary(dbTransaction, "bin"), 8);
		});
	}

	TEST(TEST_CLASS, CanMapKnownTransactionTypeWithDependentDocuments) {
		// Arrange:
		MongoTransactionRegistry registry;
		registry.registerPlugin(std::make_unique<MongoArbitraryTransactionPlugin>(DependentDocumentOptions::All));

		// Arrange:
		auto pTransaction = CreateArbitraryTransaction();
		model::TransactionElement transactionElement(*pTransaction);
		transactionElement.EntityHash = test::GenerateRandomByteArray<Hash256>();
		transactionElement.MerkleComponentHash = test::GenerateRandomByteArray<Hash256>();
		transactionElement.OptionalExtractedAddresses = test::GenerateRandomUnresolvedAddressSetPointer(3);
		auto metadata = MongoTransactionMetadata(transactionElement, Height(123), 234);

		// Act:
		auto dbModels = ToDbDocuments(*pTransaction, metadata, registry);

		// Assert:
		ASSERT_EQ(4u, dbModels.size());
		AssertTransaction(dbModels[0], *pTransaction, metadata, 2, [](const auto& dbTransaction) {
			EXPECT_EQ(0x12u, test::GetUint32(dbTransaction, "alpha"));
			EXPECT_EQ(0x65u, test::GetUint32(dbTransaction, "zeta"));
		});
		AssertSingleValueDocument(dbModels[1], "sum", 0x12 + 0x65);
		AssertSingleValueDocument(dbModels[2], "diff", 0x65 - 0x12);
		AssertSingleValueDocument(dbModels[3], "prod", 0x12 * 0x65);
	}
}}}
