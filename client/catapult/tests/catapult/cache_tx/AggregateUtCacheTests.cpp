/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "catapult/cache_tx/AggregateUtCache.h"
#include "tests/test/cache/UtTestUtils.h"
#include "tests/test/other/mocks/MockUtChangeSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AggregateUtCacheTests

	namespace {
		// region basic mocks

		class UnsupportedUtCacheModifier : public UtCacheModifier {
		public:
			size_t size() const override {
				CATAPULT_THROW_RUNTIME_ERROR("size - not supported in mock");
			}

			utils::FileSize memorySize() const override {
				CATAPULT_THROW_RUNTIME_ERROR("memorySize - not supported in mock");
			}

			bool add(const model::TransactionInfo&) override {
				CATAPULT_THROW_RUNTIME_ERROR("add - not supported in mock");
			}

			model::TransactionInfo remove(const Hash256&) override {
				CATAPULT_THROW_RUNTIME_ERROR("remove - not supported in mock");
			}

			utils::FileSize memorySizeForAccount(const Key&) const override {
				CATAPULT_THROW_RUNTIME_ERROR("memorySizeForAccount - not supported in mock");
			}

			std::vector<model::TransactionInfo> removeAll() override {
				CATAPULT_THROW_RUNTIME_ERROR("removeAll - not supported in mock");
			}
		};

		template<typename TUtCacheModifier>
		using MockUtCache = test::MockTransactionsCache<UtCache, TUtCacheModifier, UtCacheModifierProxy>;

		// endregion

		struct UtTraits {
			using CacheType = UtCache;
			using ChangeSubscriberType = mocks::MockUtChangeSubscriber;
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
			static mocks::UtFlushInfo CreateFlushInfo(size_t numAdds, size_t numRemoves) {
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

	DEFINE_AGGREGATE_TRANSACTIONS_CACHE_TESTS(TEST_CLASS, BasicTestsUtTraits)

	// endregion

	// region count

	namespace {
		class MockMemorySizeForAccountUtCacheModifier : public UnsupportedUtCacheModifier {
		public:
			MockMemorySizeForAccountUtCacheModifier(size_t& numMemorySizeCalls, std::vector<Key>& keys)
					: m_numMemorySizeCalls(numMemorySizeCalls)
					, m_keys(keys) {
				m_numMemorySizeCalls = 0;
			}

		public:
			utils::FileSize memorySizeForAccount(const Key& key) const override {
				m_keys.push_back(key);
				return utils::FileSize::FromKilobytes(++m_numMemorySizeCalls);
			}

		private:
			size_t& m_numMemorySizeCalls;
			std::vector<Key>& m_keys;
		};
	}

	TEST(TEST_CLASS, MemorySizeForAccountDelegatesToCache) {
		// Arrange:
		size_t numMemorySizeCalls;
		std::vector<Key> keys;
		auto key = test::GenerateRandomByteArray<Key>();
		TestContext<MockMemorySizeForAccountUtCacheModifier> context(numMemorySizeCalls, keys);

		// Act:
		auto memorySize = context.aggregate().modifier().memorySizeForAccount(key);

		// Assert: check ut cache modifier was called as expected
		EXPECT_EQ(utils::FileSize::FromKilobytes(1), memorySize);
		EXPECT_EQ(1u, numMemorySizeCalls);
		EXPECT_EQ(std::vector<Key>({ key }), keys);
	}

	// endregion

	// region removeAll

	namespace {
		class MockRemoveAllUtCacheModifier : public UnsupportedUtCacheModifier {
		public:
			MockRemoveAllUtCacheModifier(size_t& numRemoveAllCalls, std::vector<model::TransactionInfo>&& transactionInfos)
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
		EXPECT_EQ(mocks::UtFlushInfo({ 0u, 0u }), context.subscriber().flushInfos()[0]);
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
		EXPECT_EQ(mocks::UtFlushInfo({ 0u, 5u }), context.subscriber().flushInfos()[0]);
	}

	// endregion
}}
