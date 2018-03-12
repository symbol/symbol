#include "catapult/cache/AggregateUtCache.h"
#include "tests/catapult/cache/test/AggregateTransactionsCacheTestUtils.h"
#include "tests/test/cache/UtTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AggregateUtCacheTests

	namespace {
		// region basic mocks

		class UnsupportedUtCacheModifier : public UtCacheModifier {
		public:
			bool add(const model::TransactionInfo&) override {
				CATAPULT_THROW_RUNTIME_ERROR("add - not supported in mock");
			}

			model::TransactionInfo remove(const Hash256&) override {
				CATAPULT_THROW_RUNTIME_ERROR("remove - not supported in mock");
			}

			std::vector<model::TransactionInfo> removeAll() override {
				CATAPULT_THROW_RUNTIME_ERROR("removeAll - not supported in mock");
			}
		};

		template<typename TUtCacheModifier>
		using MockUtCache = test::MockTransactionsCache<UtCache, TUtCacheModifier, UtCacheModifierProxy>;

		struct FlushInfo {
		public:
			size_t NumAdds;
			size_t NumRemoves;

		public:
			constexpr bool operator==(const FlushInfo& rhs) const {
				return NumAdds == rhs.NumAdds && NumRemoves == rhs.NumRemoves;
			}
		};

		class MockUtChangeSubscriber : public test::MockTransactionsChangeSubscriber<UtChangeSubscriber, FlushInfo> {
		public:
			void notifyAdds(const TransactionInfos& transactionInfos) override {
				for (const auto& transactionInfo : transactionInfos)
					m_addedInfos.push_back(transactionInfo.copy());
			}

			void notifyRemoves(const TransactionInfos& transactionInfos) override {
				for (const auto& transactionInfo : transactionInfos)
					m_removedInfos.push_back(transactionInfo.copy());
			}

		private:
			FlushInfo createFlushInfo() const override {
				return { m_addedInfos.size(), m_removedInfos.size() };
			}
		};

		// endregion

		struct UtTraits {
			using CacheType = UtCache;
			using ChangeSubscriberType = MockUtChangeSubscriber;
			using UnsupportedChangeSubscriberType = test::UnsupportedUtChangeSubscriber<test::UnsupportedFlushBehavior::Throw>;

			template<typename TModifier>
			using MockCacheType = MockUtCache<TModifier>;

			static constexpr auto CreateAggregateCache = CreateAggregateUtCache;
		};

		template<typename TModifier>
		using TestContext = test::TransactionsCacheTestContext<MockUtCache<TModifier>, UtTraits>;

		struct BasicTestsUtTraits {
		public:
			using CacheTraitsType = UtTraits;
			using UnsupportedCacheModifierType = UnsupportedUtCacheModifier;
			using TransactionInfoType = model::TransactionInfo;

			template<typename TModifier>
			using TestContextType = TestContext<TModifier>;

		public:
			static FlushInfo CreateFlushInfo(size_t numAdds, size_t numRemoves) {
				return { numAdds, numRemoves };
			}

			static TransactionInfoType Copy(const TransactionInfoType& info) {
				return info.copy();
			}

			static const model::TransactionInfo& ToSubscriberInfo(const model::TransactionInfo& transactionInfo) {
				return transactionInfo;
			}
		};
	}

	// region basic tests (add / remove / flush)

	DEFINE_AGGREGATE_TRANSACTIONS_CACHE_TESTS(TEST_CLASS, BasicTestsUtTraits);

	// endregion

	// region removeAll

	namespace {
		class MockRemoveAllUtCacheModifier : public UnsupportedUtCacheModifier {
		public:
			explicit MockRemoveAllUtCacheModifier(size_t& numRemoveAllCalls, std::vector<model::TransactionInfo>&& transactionInfos)
					: m_numRemoveAllCalls(numRemoveAllCalls)
					, m_transactionInfos(std::move(transactionInfos)) {
				m_numRemoveAllCalls = 0;
			}

		public:
			std::vector<model::TransactionInfo> removeAll() override {
				++m_numRemoveAllCalls;
				return std::move(m_transactionInfos);
			}

		private:
			size_t& m_numRemoveAllCalls;
			std::vector<model::TransactionInfo> m_transactionInfos;
		};
	}

	TEST(TEST_CLASS, RemoveAllDelegatesToCacheOnlyWhenCacheIsEmpty) {
		// Arrange:
		size_t numRemoveAllCalls;
		TestContext<MockRemoveAllUtCacheModifier> context(numRemoveAllCalls, std::vector<model::TransactionInfo>());

		// Act:
		auto removedInfos = context.aggregate().modifier().removeAll();

		// Assert:
		EXPECT_TRUE(removedInfos.empty());

		// - check ut cache modifier was called as expected
		EXPECT_EQ(1u, numRemoveAllCalls);

		// - check subscriber
		ASSERT_EQ(1u, context.subscriber().flushInfos().size());
		EXPECT_EQ(FlushInfo({ 0u, 0u }), context.subscriber().flushInfos()[0]);
	}

	TEST(TEST_CLASS, RemoveAllDelegatesToCacheAndSubscriberWhenCacheIsNotEmpty) {
		// Arrange:
		size_t numRemoveAllCalls;
		auto utInfos = test::CreateTransactionInfos(5);
		TestContext<MockRemoveAllUtCacheModifier> context(numRemoveAllCalls, test::CopyTransactionInfos(utInfos));

		// Act:
		auto removedInfos = context.aggregate().modifier().removeAll();

		// Assert:
		ASSERT_EQ(5u, removedInfos.size());
		for (auto i = 0u; i < utInfos.size(); ++i)
			test::AssertEqual(utInfos[i], removedInfos[i], "info from remove all " + std::to_string(i));

		// - check ut cache modifier was called as expected
		EXPECT_EQ(1u, numRemoveAllCalls);

		// - check subscriber
		ASSERT_EQ(5u, context.subscriber().removedInfos().size());
		test::AssertEquivalent(utInfos, context.subscriber().removedInfos(), "subscriber infos");

		ASSERT_EQ(1u, context.subscriber().flushInfos().size());
		EXPECT_EQ(FlushInfo({ 0u, 5u }), context.subscriber().flushInfos()[0]);
	}

	// endregion
}}
