#include "src/StorageAwareUtCache.h"
#include "catapult/io/TransactionStorage.h"
#include "tests/test/cache/UnconfirmedTransactionsTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		constexpr size_t Num_Transaction_Infos = 3;

		struct Params {
			Params()
					: NumRemoveAllCalls(0)
					, NumCommitCalls(0)
					, Timestamp(0)
			{}

			std::vector<model::TransactionInfo> SavedTransactionInfos;
			std::vector<model::TransactionInfo> RemovedTransactionInfos;
			std::vector<Hash256> Hashes;
			size_t NumRemoveAllCalls;
			size_t NumCommitCalls;
			catapult::Timestamp Timestamp;
		};

		class MockTransactionStorage : public io::TransactionStorage {
		public:
			explicit MockTransactionStorage(Params& params)
					: m_params(params)
					, m_saveTransactionReturnValue(true)
			{}

		public:
			bool saveTransaction(const model::TransactionInfo& transactionInfo) override {
				m_params.SavedTransactionInfos.emplace_back(transactionInfo.copy());
				return m_saveTransactionReturnValue;
			}

			void removeTransaction(const Hash256& hash) override {
				m_params.Hashes.push_back(hash);
			}

			void removeTransactions(const std::vector<model::TransactionInfo>& transactionInfos) override {
				++m_params.NumRemoveAllCalls;
				for (const auto& info : transactionInfos)
					m_params.RemovedTransactionInfos.emplace_back(info.copy());
			}

			void pruneTransactions(Timestamp timestamp) override {
				m_params.Timestamp = timestamp;
			}

			void commit() override {
				++m_params.NumCommitCalls;
			}

		public:
			void setSaveTransactionReturnValue(bool value) {
				m_saveTransactionReturnValue = value;
			}

		private:
			Params& m_params;
			bool m_saveTransactionReturnValue;
		};

		class MockUnconfirmedTransactionsCacheModifier : public cache::UtCacheModifier {
		public:
			explicit MockUnconfirmedTransactionsCacheModifier(Params& params)
					: m_params(params)
					, m_addReturnValue(true)
			{}

		public:
			bool add(model::TransactionInfo&& transactionInfo) override {
				m_params.SavedTransactionInfos.push_back(std::move(transactionInfo));
				return m_addReturnValue;
			}

			void remove(const Hash256& hash) override {
				m_params.Hashes.push_back(hash);
				m_params.NumRemoveAllCalls++;
			}

			std::vector<model::TransactionInfo> removeAll() override {
				m_params.NumRemoveAllCalls++;
				return test::CreateTransactionInfos(Num_Transaction_Infos);
			}

			void prune(Timestamp timestamp) override {
				m_params.Timestamp = timestamp;
			}

		public:
			void setAddReturnValue(bool value) {
				m_addReturnValue = value;
			}

		private:
			Params& m_params;
			bool m_addReturnValue;
		};

		class MockUnconfirmedTransactionsCache : public cache::UtCache {
		public:
			explicit MockUnconfirmedTransactionsCache(Params& params)
					: m_pModifier(std::make_unique<MockUnconfirmedTransactionsCacheModifier>(params))
			{}

		public:
			cache::UtCacheModifierProxy modifier() override {
				if (nullptr == m_pModifier)
					CATAPULT_THROW_RUNTIME_ERROR("modifier() called more than once");

				return cache::UtCacheModifierProxy(std::move(m_pModifier));
			}

		public:
			void setAddReturnValue(bool value) {
				m_pModifier->setAddReturnValue(value);
			}

		private:
			std::unique_ptr<MockUnconfirmedTransactionsCacheModifier> m_pModifier;
		};

		struct TestContext {
		public:
			TestContext()
					: pCache(std::make_shared<MockUnconfirmedTransactionsCache>(CacheParams))
					, pStorage(std::make_shared<MockTransactionStorage>(StorageParams))
					, DbAwareCache(pCache, pStorage)
			{}

		public:
			Params CacheParams;
			Params StorageParams;
			std::shared_ptr<MockUnconfirmedTransactionsCache> pCache;
			std::shared_ptr<MockTransactionStorage> pStorage;
			StorageAwareUtCache DbAwareCache;
		};

		void AssertEqualTransactionInfoDeadlines(
				const std::vector<model::TransactionInfo>& expectedInfos,
				const std::vector<model::TransactionInfo>& actualInfos) {
			ASSERT_EQ(expectedInfos.size(), actualInfos.size());
			for (auto i = 0u; i < expectedInfos.size(); ++i)
				EXPECT_EQ(expectedInfos[i].pEntity->Deadline, actualInfos[i].pEntity->Deadline) << "transaction info at " << i;
		}
	}

	// region add

	namespace {
		void AssertReturnValue(bool cacheReturnValue, bool storageReturnValue, bool expectedReturnValue) {
			// Arrange:
			TestContext context;
			context.pCache->setAddReturnValue(cacheReturnValue);
			context.pStorage->setSaveTransactionReturnValue(storageReturnValue);
			auto modifier = context.DbAwareCache.modifier();
			model::TransactionInfo transactionInfo;

			// Act:
			auto result = modifier.add(std::move(transactionInfo));

			// Assert:
			EXPECT_EQ(expectedReturnValue, result);
		}
	}

	TEST(StorageAwareUtCacheTests, AddDelegatesToCacheAndStorage) {
		// Arrange:
		TestContext context;
		auto pTransaction = test::GenerateRandomTransaction();
		const auto& transaction = *pTransaction;
		auto entityHash = test::GenerateRandomData<Hash256_Size>();
		auto merkleComponentHash = test::GenerateRandomData<Hash256_Size>();
		auto transactionInfo = model::TransactionInfo(std::move(pTransaction), entityHash, merkleComponentHash);

		// Act:
		auto result = context.DbAwareCache.modifier().add(std::move(transactionInfo));

		// Assert:
		auto& cacheInfos = context.CacheParams.SavedTransactionInfos;
		auto& storageInfos = context.StorageParams.SavedTransactionInfos;
		EXPECT_TRUE(result);

		ASSERT_EQ(1u, cacheInfos.size());
		EXPECT_EQ(transaction, *cacheInfos[0].pEntity);
		EXPECT_EQ(entityHash, cacheInfos[0].EntityHash);
		EXPECT_EQ(merkleComponentHash, cacheInfos[0].MerkleComponentHash);

		ASSERT_EQ(1u, storageInfos.size());
		EXPECT_EQ(transaction, *storageInfos[0].pEntity);
		EXPECT_EQ(entityHash, storageInfos[0].EntityHash);
		EXPECT_EQ(merkleComponentHash, storageInfos[0].MerkleComponentHash);
	}

	TEST(StorageAwareUtCacheTests, AddReturnsTrueIfAndOnlyIfBothCacheAndStorageAddReturnTrue) {
		// Assert:
		AssertReturnValue(true, true, true);
		AssertReturnValue(true, false, false);
		AssertReturnValue(false, true, false);
		AssertReturnValue(false, false, false);
	}

	// endregion

	// region remove

	TEST(StorageAwareUtCacheTests, RemoveDelegatesToCacheAndStorage) {
		// Arrange:
		TestContext context;
		auto hash = test::GenerateRandomData<Hash256_Size>();

		// Act:
		context.DbAwareCache.modifier().remove(hash);

		// Assert:
		ASSERT_EQ(1u, context.CacheParams.Hashes.size());
		EXPECT_EQ(hash, context.CacheParams.Hashes[0]);
		ASSERT_EQ(1u, context.StorageParams.Hashes.size());
		EXPECT_EQ(hash, context.StorageParams.Hashes[0]);
	}

	// endregion

	// region removeAll

	TEST(StorageAwareUtCacheTests, RemoveAllDelegatesToCacheAndStorage) {
		// Arrange:
		TestContext context;

		// Act:
		auto removedInfos = context.DbAwareCache.modifier().removeAll();

		// Assert:
		EXPECT_EQ(1u, context.CacheParams.NumRemoveAllCalls);
		EXPECT_EQ(1u, context.StorageParams.NumRemoveAllCalls);
		AssertEqualTransactionInfoDeadlines(removedInfos, context.StorageParams.RemovedTransactionInfos);
	}

	// endregion

	// region prune

	TEST(StorageAwareUtCacheTests, PruneDelegatesToCacheAndStorage) {
		// Arrange:
		TestContext context;
		Timestamp timestamp(123);

		// Act:
		context.DbAwareCache.modifier().prune(timestamp);

		// Assert:
		EXPECT_EQ(timestamp, context.CacheParams.Timestamp);
		EXPECT_EQ(timestamp, context.StorageParams.Timestamp);
	}

	// endregion

	// region commit

	TEST(StorageAwareUtCacheTests, ModifierDtorDelegatesToStorageCommit) {
		// Arrange:
		TestContext context;

		// Sanity:
		EXPECT_EQ(0u, context.StorageParams.NumCommitCalls);

		// Act: modifier should die in the block
		{
			context.DbAwareCache.modifier();
		}

		// Assert:
		EXPECT_EQ(1u, context.StorageParams.NumCommitCalls);
	}

	// endregion
}}}
