#include "catapult/local/p2p/HashPredicateFactory.h"
#include "catapult/cache/MemoryUtCache.h"
#include "tests/catapult/local/p2p/utils/PeerCacheTestUtils.h"
#include "tests/test/cache/UnconfirmedTransactionsTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace p2p {

	namespace {
		constexpr size_t Num_TransactionInfos = 5;

		class TestContext {
		public:
			TestContext()
					: m_transactionInfos(test::CreateTransactionInfos(Num_TransactionInfos))
					, m_utCache(cache::MemoryUtCacheOptions(Num_TransactionInfos, Num_TransactionInfos))
					, m_catapultCache(test::PeerCacheFactory::Create())
					, m_factory(m_utCache, m_catapultCache)
			{}

		public:
			auto& transactionInfos() {
				return m_transactionInfos;
			}

			const auto& factory() {
				return m_factory;
			}

		public:
			void populateUtCache() {
				test::AddAll(m_utCache, test::CopyTransactionInfos(m_transactionInfos));
			}

			void populateHashCache() {
				auto delta = m_catapultCache.createDelta();
				auto& hashCacheDelta = delta.sub<cache::HashCache>();

				for (const auto& transactionInfo : m_transactionInfos)
					hashCacheDelta.insert(transactionInfo.pEntity->Deadline, transactionInfo.EntityHash);

				m_catapultCache.commit(Height());
			}

		private:
			std::vector<model::TransactionInfo> m_transactionInfos;
			cache::MemoryUtCache m_utCache;
			cache::CatapultCache m_catapultCache;
			HashPredicateFactory m_factory;
		};
	}

	// region UnknownTransactionPredicate

	TEST(HashPredicateFactoryTests, UnknownTransactionPredicate_ReturnsFalseForBlockType) {
		// Arrange:
		TestContext context;
		auto predicate = context.factory().createUnknownTransactionPredicate();

		// Act:
		auto isMatch = predicate(model::BasicEntityType::Block, Timestamp(12345), test::GenerateRandomData<Hash256_Size>());

		// Assert:
		EXPECT_FALSE(isMatch);
	}

	TEST(HashPredicateFactoryTests, UnknownTransactionPredicate_ReturnsFalseIfEntityIsContainedInUtCache) {
		// Arrange:
		TestContext context;
		context.populateUtCache();
		auto predicate = context.factory().createUnknownTransactionPredicate();

		// Act:
		auto isMatch = predicate(model::BasicEntityType::Transaction, Timestamp(12345), context.transactionInfos()[2].EntityHash);

		// Assert:
		EXPECT_FALSE(isMatch);
	}

	TEST(HashPredicateFactoryTests, UnknownTransactionPredicate_ReturnsFalseIfEntityIsContainedInHashCache) {
		// Arrange:
		TestContext context;
		context.populateHashCache();
		auto predicate = context.factory().createUnknownTransactionPredicate();

		// Act:
		auto isMatch = predicate(
				model::BasicEntityType::Transaction,
				context.transactionInfos()[2].pEntity->Deadline,
				context.transactionInfos()[2].EntityHash);

		// Assert:
		EXPECT_FALSE(isMatch);
	}

	TEST(HashPredicateFactoryTests, UnknownTransactionPredicate_ReturnsTrueIfEntityIsNeitherContainedInUtCacheNorInHashCache) {
		// Arrange:
		TestContext context;
		context.populateUtCache();
		context.populateHashCache();
		auto predicate = context.factory().createUnknownTransactionPredicate();

		// Act:
		auto isMatch = predicate(model::BasicEntityType::Transaction, Timestamp(12345), test::GenerateRandomData<Hash256_Size>());

		// Assert:
		EXPECT_TRUE(isMatch);
	}

	// endregion

	// region KnownHashPredicate

	TEST(HashPredicateFactoryTests, KnownHashPredicate_ReturnsTrueIfEntityIsContainedInUtCache) {
		// Arrange:
		TestContext context;
		context.populateUtCache();
		auto predicate = context.factory().createKnownHashPredicate();

		// Act:
		auto isMatch = predicate(Timestamp(12345), context.transactionInfos()[2].EntityHash);

		// Assert:
		EXPECT_TRUE(isMatch);
	}

	TEST(HashPredicateFactoryTests, KnownHashPredicate_ReturnsTrueIfEntityIsContainedInHashCache) {
		// Arrange:
		TestContext context;
		context.populateHashCache();
		auto predicate = context.factory().createKnownHashPredicate();

		// Act:
		auto isMatch = predicate(context.transactionInfos()[2].pEntity->Deadline, context.transactionInfos()[2].EntityHash);

		// Assert:
		EXPECT_TRUE(isMatch);
	}

	TEST(HashPredicateFactoryTests, KnownHashPredicate_ReturnsFalseIfEntityIsNeitherContainedInUtCacheNorInHashCache) {
		// Arrange:
		TestContext context;
		auto predicate = context.factory().createKnownHashPredicate();

		// Act:
		auto isMatch = predicate(Timestamp(12345), test::GenerateRandomData<Hash256_Size>());

		// Assert:
		EXPECT_FALSE(isMatch);
	}

	// endregion
}}}
