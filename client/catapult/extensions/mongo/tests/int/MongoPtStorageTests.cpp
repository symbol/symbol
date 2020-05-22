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

#include "mongo/src/MongoPtStorage.h"
#include "catapult/model/Cosignature.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionStorageTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

#define TEST_CLASS MongoPtStorageTests

	namespace {
		constexpr auto Pt_Collection_Name = "partialTransactions";
		constexpr size_t Num_Transactions = 10;

		class PtStorageContext {
		public:
			explicit PtStorageContext(
					size_t numTransactionInfos,
					test::DbInitializationType dbInitializationType = test::DbInitializationType::Reset)
					: PtStorageContext(numTransactionInfos, mocks::CreateMockTransactionMongoPlugin(), dbInitializationType)
			{}

			PtStorageContext(
					size_t numTransactionInfos,
					std::unique_ptr<MongoTransactionPlugin>&& pTransactionPlugin,
					test::DbInitializationType dbInitializationType = test::DbInitializationType::Reset)
					: m_pStorage(test::CreateMongoStorage<cache::PtChangeSubscriber>(
							std::move(pTransactionPlugin),
							dbInitializationType,
							MongoErrorPolicy::Mode::Strict,
							CreateMongoPtStorage))
					, m_transactionInfos(test::CreateTransactionInfos(numTransactionInfos))
			{}

		public:
			std::string collectionName() {
				return Pt_Collection_Name;
			}

			std::vector<model::TransactionInfo>& transactionInfos() {
				return m_transactionInfos;
			}

			void saveTransaction(const model::TransactionInfo& transactionInfo) {
				cache::PtChangeSubscriber::TransactionInfos transactionInfos;
				transactionInfos.emplace(transactionInfo.copy());
				saveTransactions(transactionInfos);
			}

			void saveTransactions(const cache::PtChangeSubscriber::TransactionInfos& transactionInfos) {
				m_pStorage->notifyAddPartials(transactionInfos);
			}

			void saveCosignature(const model::TransactionInfo& parentTransactionInfo, const model::Cosignature& cosignature) {
				m_pStorage->notifyAddCosignature(parentTransactionInfo, cosignature);
			}

			void saveCosignatures(const model::TransactionInfo& transactionInfo, const std::vector<model::Cosignature>& cosignatures) {
				for (const auto& cosignature : cosignatures)
					saveCosignature(transactionInfo, cosignature);
			}

			void removeTransaction(const model::TransactionInfo& transactionInfo) {
				cache::PtChangeSubscriber::TransactionInfos transactionInfos;
				transactionInfos.emplace(transactionInfo.copy());
				removeTransactions(transactionInfos);
			}

			void removeTransactions(const cache::PtChangeSubscriber::TransactionInfos& transactionInfos) {
				m_pStorage->notifyRemovePartials(transactionInfos);
			}

			void flush() {
				m_pStorage->flush();
			}

			void seedDatabase() {
				for (const auto& transactionInfo : m_transactionInfos)
					saveTransaction(transactionInfo);

				// Sanity:
				test::AssertCollectionSize(collectionName(), m_transactionInfos.size());
			}

		private:
			std::shared_ptr<cache::PtChangeSubscriber> m_pStorage;
			std::vector<model::TransactionInfo> m_transactionInfos;
		};

		void AssertCosignatures(const Hash256& parentHash, const std::vector<model::Cosignature>& cosignatures) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];
			auto collection = database[Pt_Collection_Name];
			auto txFilter = document() << "meta.hash" << mappers::ToBinary(parentHash) << finalize;
			auto optionalValue = collection.find_one(txFilter.view());

			ASSERT_TRUE(optionalValue.has_value());

			auto dbDoc = optionalValue.value();
			auto dbTransaction = dbDoc.view()["transaction"];
			auto dbCosignatures = dbTransaction["cosignatures"].get_array().value;
			ASSERT_EQ(cosignatures.size(), test::GetFieldCount(dbCosignatures));

			test::AssertEqualCosignatures(cosignatures, dbCosignatures);
		}

		void AssertNoTransaction(const Hash256& parentHash) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];
			auto collection = database[Pt_Collection_Name];
			auto txFilter = document() << "meta.hash" << mappers::ToBinary(parentHash) << finalize;
			auto optionalValue = collection.find_one(txFilter.view());

			ASSERT_FALSE(optionalValue.has_value());
		}
	}

	// region basic storage tests

	DEFINE_MONGO_TRANSACTION_STORAGE_SAVE_TESTS(PtStorage)
	DEFINE_MONGO_TRANSACTION_STORAGE_REMOVE_TESTS(PtStorage)
	DEFINE_MONGO_TRANSACTION_STORAGE_FLUSH_TESTS(PtStorage)

	// endregion

	// region additional flush tests

	TEST(TEST_CLASS, FlushResetsQueuedCosignatures) {
		// Arrange:
		PtStorageContext context(Num_Transactions);
		context.seedDatabase();
		auto transactionInfo = test::CreateRandomTransactionInfo();
		context.saveTransaction(transactionInfo);
		auto cosignature = test::CreateRandomDetachedCosignature();
		context.saveCosignature(transactionInfo, cosignature);
		context.flush();

		// Sanity:
		test::AssertCollectionSize(context.collectionName(), Num_Transactions + 1);
		AssertCosignatures(transactionInfo.EntityHash, { cosignature });

		// Act:
		context.flush();

		// Assert: if cosignatures map was not reset during flush, a second flush would add the same cosignatures a second time
		AssertCosignatures(transactionInfo.EntityHash, { cosignature });
	}

	// endregion

	// region notifyAddCosignature

	TEST(TEST_CLASS, CanSaveSingleCosignature) {
		// Arrange:
		PtStorageContext context(Num_Transactions);
		context.seedDatabase();
		auto transactionInfo = test::CreateRandomTransactionInfo();
		context.saveTransaction(transactionInfo);
		auto cosignature = test::CreateRandomDetachedCosignature();

		// Act:
		context.saveCosignature(transactionInfo, cosignature);
		context.flush();

		// Assert:
		AssertCosignatures(transactionInfo.EntityHash, { cosignature });
	}

	TEST(TEST_CLASS, CanSaveMultipleCosignatures) {
		// Arrange:
		PtStorageContext context(Num_Transactions);
		context.seedDatabase();
		auto transactionInfo = test::CreateRandomTransactionInfo();
		context.saveTransaction(transactionInfo);
		auto cosignatures = test::GenerateRandomDataVector<model::Cosignature>(3);

		// Act:
		context.saveCosignatures(transactionInfo, cosignatures);
		context.flush();

		// Assert:
		AssertCosignatures(transactionInfo.EntityHash, cosignatures);
	}

	TEST(TEST_CLASS, CanSaveMultipleCosignaturesForMultipleTransactions) {
		// Arrange:
		auto numAdditionalTransactions = 3u;
		PtStorageContext context(Num_Transactions);
		context.seedDatabase();
		auto transactionInfos = test::CreateTransactionInfos(numAdditionalTransactions);
		for (const auto& transactionInfo : transactionInfos)
			context.saveTransaction(transactionInfo);

		std::vector<std::vector<model::Cosignature>> cosignaturesGroups{
			test::GenerateRandomDataVector<model::Cosignature>(2),
			test::GenerateRandomDataVector<model::Cosignature>(4),
			test::GenerateRandomDataVector<model::Cosignature>(3)
		};

		// Act:
		for (auto i = 0u; i < numAdditionalTransactions; ++i)
			context.saveCosignatures(transactionInfos[i], cosignaturesGroups[i]);

		context.flush();

		// Assert:
		for (auto i = 0u; i < numAdditionalTransactions; ++i)
			AssertCosignatures(transactionInfos[i].EntityHash, cosignaturesGroups[i]);
	}

	TEST(TEST_CLASS, CanSaveCosignatureForFlushedTransaction) {
		// Arrange:
		PtStorageContext context(Num_Transactions);
		context.seedDatabase();

		// - save (remove from pending changes) the partial transaction
		auto transactionInfo = test::CreateRandomTransactionInfo();
		context.saveTransaction(transactionInfo);

		auto cosignature = test::CreateRandomDetachedCosignature();

		// Act:
		context.saveCosignature(transactionInfo, cosignature);
		context.flush();

		// Assert:
		AssertCosignatures(transactionInfo.EntityHash, { cosignature });
	}

	TEST(TEST_CLASS, SaveCosignatureAndThenRemovingTransactionRemovesBoth) {
		// Arrange:
		PtStorageContext context(Num_Transactions);
		context.seedDatabase();
		auto transactionInfo = test::CreateRandomTransactionInfo();
		context.saveTransaction(transactionInfo);
		auto cosignature = test::CreateRandomDetachedCosignature();

		// Act:
		context.saveCosignature(transactionInfo, cosignature);
		context.removeTransaction(transactionInfo);
		context.flush();

		// Assert:
		AssertNoTransaction(transactionInfo.EntityHash);
	}

	// endregion
}}
