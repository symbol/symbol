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

#pragma once
#include "UnsupportedTransactionsChangeSubscribers.h"
#include "catapult/utils/FileSize.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include <unordered_map>

namespace catapult { namespace test {

	// region MockTransactionsCache

	/// Mock transactions cache.
	template<typename TCache, typename TCacheModifier, typename TCacheModifierProxy>
	class MockTransactionsCache : public TCache {
	public:
		/// Creates a cache around \a args.
		template<typename... TArgs>
		explicit MockTransactionsCache(TArgs&&... args) : m_pModifier(std::make_unique<TCacheModifier>(std::forward<TArgs>(args)...))
		{}

	public:
		TCacheModifierProxy modifier() override {
			return TCacheModifierProxy(std::move(m_pModifier));
		}

	private:
		std::unique_ptr<TCacheModifier> m_pModifier;
	};

	// endregion

	// region MockTransactionsChangeSubscriber

	/// Mock transactions change subscriber.
	template<typename TChangeSubscriber, typename TFlushInfo>
	class MockTransactionsChangeSubscriber : public TChangeSubscriber {
	public:
		/// Gets all captured added infos.
		const std::vector<model::TransactionInfo>& addedInfos() const {
			return m_addedInfos;
		}

		/// Gets all captured removed infos.
		const std::vector<model::TransactionInfo>& removedInfos() const {
			return m_removedInfos;
		}

		/// Gets all captured flush infos.
		const std::vector<TFlushInfo>& flushInfos() const {
			return m_flushInfos;
		}

	public:
		/// Clears all added, removed and flush infos.
		void reset() {
			m_addedInfos.clear();
			m_removedInfos.clear();
			m_flushInfos.clear();
		}

	public:
		void flush() override {
			m_flushInfos.push_back(createFlushInfo());
		}

	private:
		virtual TFlushInfo createFlushInfo() const = 0;

	protected:
		std::vector<model::TransactionInfo> m_addedInfos;
		std::vector<model::TransactionInfo> m_removedInfos;

	private:
		std::vector<TFlushInfo> m_flushInfos;
	};

	// endregion

	// region TransactionsCacheTestContext

	template<typename TMockCache, typename TCacheTraits>
	class TransactionsCacheTestContext {
	private:
		using CacheType = typename TCacheTraits::CacheType;
		using ChangeSubscriberType = typename TCacheTraits::ChangeSubscriberType;

	public:
		/// Creates a context around \a args.
		template<typename... TArgs>
		TransactionsCacheTestContext(TArgs&&... args)
				: m_cache(std::forward<TArgs>(args)...)
				, m_pSubscriber(std::make_unique<ChangeSubscriberType>())
				, m_pSubscriberRaw(m_pSubscriber.get())
				, m_pAggregate(TCacheTraits::CreateAggregateCache(m_cache, std::move(m_pSubscriber)))
		{}

	public:
		/// Gets the subscriber.
		auto& subscriber() {
			return *m_pSubscriberRaw;
		}

		/// Gets the aggregate.
		auto& aggregate() {
			return *m_pAggregate;
		}

	private:
		TMockCache m_cache;
		std::unique_ptr<ChangeSubscriberType> m_pSubscriber; // notice that this is moved into m_pAggregate
		ChangeSubscriberType* m_pSubscriberRaw;
		std::unique_ptr<CacheType> m_pAggregate;
	};

	// endregion

	/// Container of basic aggregate transactions cache tests.
	template<typename TTraits>
	struct BasicAggregateTransactionsCacheTests {
	public:
		using UnsupportedCacheModifierType = typename TTraits::UnsupportedCacheModifierType;
		using TransactionInfoType = typename TTraits::TransactionInfoType;

		template<typename TModifier>
		using TestContext = typename TTraits::template TestContextType<TModifier>;

	private:
		// region mock cache modifiers

		class MockSizeCacheModifier : public UnsupportedCacheModifierType {
		public:
			explicit MockSizeCacheModifier(size_t size) : m_size(size)
			{}

		public:
			size_t size() const override {
				return m_size;
			}

		private:
			size_t m_size;
		};

		class MockMemorySizeCacheModifier : public UnsupportedCacheModifierType {
		public:
			explicit MockMemorySizeCacheModifier(size_t size) : m_size(size)
			{}

		public:
			utils::FileSize memorySize() const override {
				return utils::FileSize::FromBytes(m_size);
			}

		private:
			size_t m_size;
		};

		template<bool AddResult>
		class MockAddCacheModifier : public UnsupportedCacheModifierType {
		public:
			explicit MockAddCacheModifier(std::vector<TransactionInfoType>& transactionInfos) : m_transactionInfos(transactionInfos)
			{}

		public:
			bool add(const TransactionInfoType& info) override {
				m_transactionInfos.push_back(TTraits::Copy(info));
				return AddResult;
			}

		private:
			std::vector<TransactionInfoType>& m_transactionInfos;
		};

		class MockRemoveCacheModifier : public UnsupportedCacheModifierType {
		public:
			MockRemoveCacheModifier(std::vector<Hash256>& hashes, const TransactionInfoType& info)
					: m_hashes(hashes)
					, m_info(TTraits::Copy(info))
			{}

		public:
			TransactionInfoType remove(const Hash256& hash) override {
				m_hashes.push_back(hash);
				return TTraits::Copy(m_info);
			}

		private:
			std::vector<Hash256>& m_hashes;
			TransactionInfoType m_info;
		};

		class MockAddRemoveCacheModifier : public UnsupportedCacheModifierType {
		public:
			using HashTransactionInfoMap = std::unordered_map<Hash256, TransactionInfoType, utils::ArrayHasher<Hash256>>;

		public:
			explicit MockAddRemoveCacheModifier(const HashTransactionInfoMap& hashTransactionInfoMap)
					: m_hashTransactionInfoMap(hashTransactionInfoMap)
			{}

		public:
			bool add(const TransactionInfoType&) override {
				// allow all adds to proceed
				return true;
			}

			TransactionInfoType remove(const Hash256& hash) override {
				// allow all removes to proceed
				auto iter = m_hashTransactionInfoMap.find(hash);
				return m_hashTransactionInfoMap.cend() == iter ? TransactionInfoType() : TTraits::Copy(iter->second);
			}

		private:
			const HashTransactionInfoMap& m_hashTransactionInfoMap;
		};

		// endregion

		// region aggregate transaction cache - size / memorySize

	public:
		static void AssertSizeDelegatesToCache() {
			// Arrange:
			TestContext<MockSizeCacheModifier> context(14);

			// Act:
			auto size = context.aggregate().modifier().size();

			// Assert:
			EXPECT_EQ(14u, size);
		}

		static void AssertMemorySizeDelegatesToCache() {
			// Arrange:
			TestContext<MockMemorySizeCacheModifier> context(14);

			// Act:
			auto size = context.aggregate().modifier().memorySize();

			// Assert:
			EXPECT_EQ(utils::FileSize::FromBytes(14), size);
		}

		// endregion

		// region aggregate transaction cache - add

	private:
		static void AssertAddDelegatesToCacheAndSubscriberAfterSuccessfulCacheAdds(size_t numAdds) {
			// Arrange:
			std::vector<TransactionInfoType> cacheInfos;
			TestContext<MockAddCacheModifier<true>> context(cacheInfos);

			auto transactionInfo = test::CreateRandomTransactionInfo();

			// Act: add via modifier, which flushes when destroyed
			std::vector<bool> results;
			{
				auto modifier = context.aggregate().modifier();
				for (auto i = 0u; i < numAdds; ++i)
					results.push_back(modifier.add(transactionInfo));
			}

			// Assert:
			ASSERT_EQ(numAdds, results.size());
			for (auto i = 0u; i < numAdds; ++i)
				EXPECT_TRUE(results[i]) << "result at " << i;

			// - check cache
			ASSERT_EQ(numAdds, cacheInfos.size());
			for (auto i = 0u; i < numAdds; ++i)
				test::AssertEqual(transactionInfo, cacheInfos[i], "cache info at " + std::to_string(i));

			// - check subscriber
			ASSERT_EQ(1u, context.subscriber().addedInfos().size());
			test::AssertEqual(TTraits::ToSubscriberInfo(transactionInfo), context.subscriber().addedInfos()[0]);

			ASSERT_EQ(1u, context.subscriber().flushInfos().size());
			EXPECT_EQ(TTraits::CreateFlushInfo(1, 0), context.subscriber().flushInfos()[0]);
		}

	public:
		/// Asserts that add delegates to subscriber after successful cache add.
		static void AssertAddDelegatesToCacheAndSubscriberAfterSuccessfulCacheAdd() {
			// Assert:
			AssertAddDelegatesToCacheAndSubscriberAfterSuccessfulCacheAdds(1);
		}

		/// Asserts that there is a single notification for redundant adds.
		static void AssertSingleNotifificationForRedundantAdds() {
			// Assert:
			AssertAddDelegatesToCacheAndSubscriberAfterSuccessfulCacheAdds(3);
		}

		/// Asserts that add does not delegate to subscriber after failed cache add.
		static void AssertAddDelegatesToOnlyCacheAfterFailedCacheAdd() {
			// Arrange:
			std::vector<TransactionInfoType> cacheInfos;
			TestContext<MockAddCacheModifier<false>> context(cacheInfos);

			auto transactionInfo = test::CreateRandomTransactionInfo();

			// Act: add via modifier, which flushes when destroyed
			auto result = context.aggregate().modifier().add(transactionInfo);

			// Assert:
			EXPECT_FALSE(result);

			// - check cache
			ASSERT_EQ(1u, cacheInfos.size());
			test::AssertEqual(transactionInfo, cacheInfos[0]);

			// - check subscriber (no adds)
			ASSERT_EQ(1u, context.subscriber().flushInfos().size());
			EXPECT_EQ(TTraits::CreateFlushInfo(0, 0), context.subscriber().flushInfos()[0]);
		}

		// endregion

		// region aggregate transaction cache - remove

	private:
		static void AssertRemoveDelegatesToCacheAndSubscriberAfterSuccessfulCacheRemoves(size_t numRemoves) {
			// Arrange:
			std::vector<Hash256> cacheHashes;
			auto cacheInfo = test::CreateRandomTransactionInfo();
			TestContext<MockRemoveCacheModifier> context(cacheHashes, cacheInfo);

			auto hash = test::GenerateRandomByteArray<Hash256>();

			// Act: remove via modifier, which flushes when destroyed
			std::vector<TransactionInfoType> removedInfos;
			{
				auto modifier = context.aggregate().modifier();
				for (auto i = 0u; i < numRemoves; ++i)
					removedInfos.push_back(modifier.remove(hash));
			}

			// Assert:
			ASSERT_EQ(numRemoves, removedInfos.size());
			for (auto i = 0u; i < numRemoves; ++i)
				test::AssertEqual(cacheInfo, removedInfos[i], "removed info at " + std::to_string(i));

			// - check cache
			ASSERT_EQ(numRemoves, cacheHashes.size());
			for (auto i = 0u; i < numRemoves; ++i)
				EXPECT_EQ(hash, cacheHashes[i]) << "cache hash at " << i;

			// - check subscriber
			ASSERT_EQ(1u, context.subscriber().removedInfos().size());
			test::AssertEqual(TTraits::ToSubscriberInfo(cacheInfo), context.subscriber().removedInfos()[0]);

			ASSERT_EQ(1u, context.subscriber().flushInfos().size());
			EXPECT_EQ(TTraits::CreateFlushInfo(0, 1), context.subscriber().flushInfos()[0]);
		}

	public:
		/// Asserts that remove delegates to subscriber after successful cache remove.
		static void AssertRemoveDelegatesToCacheAndSubscriberAfterSuccessfulCacheRemove() {
			// Assert:
			AssertRemoveDelegatesToCacheAndSubscriberAfterSuccessfulCacheRemoves(1);
		}

		/// Asserts that there is a single notification for redundant removes.
		static void AssertSingleNotifificationForRedundantRemoves() {
			// Assert:
			AssertRemoveDelegatesToCacheAndSubscriberAfterSuccessfulCacheRemoves(3);
		}

		/// Asserts that remove does not delegate to subscriber after failed cache remove.
		static void AssertRemoveDelegatesToOnlyCacheAfterFailedCacheRemove() {
			// Arrange: configure the cache to return an empty info
			std::vector<Hash256> cacheHashes;
			auto cacheInfo = TransactionInfoType();
			TestContext<MockRemoveCacheModifier> context(cacheHashes, cacheInfo);

			auto hash = test::GenerateRandomByteArray<Hash256>();

			// Act:
			auto removedInfo = context.aggregate().modifier().remove(hash);

			// Assert:
			EXPECT_FALSE(!!removedInfo);

			// - check cache
			ASSERT_EQ(1u, cacheHashes.size());
			EXPECT_EQ(hash, cacheHashes[0]);

			// - check subscriber (no removes)
			ASSERT_EQ(1u, context.subscriber().flushInfos().size());
			EXPECT_EQ(TTraits::CreateFlushInfo(0, 0), context.subscriber().flushInfos()[0]);
		}

		// endregion

		// region aggregate transaction cache - flush

		/// Asserts that modifier destructor delegates to flush.
		static void AssertModifierDestructorDelegatesToFlush() {
			// Arrange:
			std::vector<TransactionInfoType> cacheInfos;
			TestContext<MockAddCacheModifier<false>> context(cacheInfos);

			// Sanity:
			EXPECT_EQ(0u, context.subscriber().flushInfos().size());

			// Act: modifier should be created and destroyed, which will trigger flush
			context.aggregate().modifier();

			// Assert:
			ASSERT_EQ(1u, context.subscriber().flushInfos().size());
			EXPECT_EQ(TTraits::CreateFlushInfo(0, 0), context.subscriber().flushInfos()[0]);
		}

		// endregion

		// region aggregate transaction cache - net changes

	private:
		static auto CreateSubscriberTransactionInfos(size_t count) {
			// adjust the merkle hashes so they are stripped as appropriate
			auto transactionInfos = test::CreateTransactionInfos(count);
			for (auto& transactionInfo : transactionInfos)
				transactionInfo.MerkleComponentHash = TTraits::ToSubscriberInfo(transactionInfo).MerkleComponentHash;

			return transactionInfos;
		}

		struct AddRemoveInfos {
		public:
			AddRemoveInfos(size_t numAdded, size_t numRemoved) {
				AddedInfos = CreateSubscriberTransactionInfos(numAdded);
				RemovedInfos = CreateSubscriberTransactionInfos(numRemoved);

				for (const auto& info : AddedInfos)
					RemovedInfosMap.emplace(info.EntityHash, TTraits::Copy(info));

				for (const auto& info : RemovedInfos)
					RemovedInfosMap.emplace(info.EntityHash, TTraits::Copy(info));
			}

		public:
			std::vector<model::TransactionInfo> AddedInfos;
			std::vector<model::TransactionInfo> RemovedInfos;
			typename MockAddRemoveCacheModifier::HashTransactionInfoMap RemovedInfosMap;

		public:
			template<typename TModifier>
			void addRemove(TModifier& modifier) {
				for (const auto& info : AddedInfos)
					modifier.add(info);

				for (const auto& info : RemovedInfos)
					modifier.remove(info.EntityHash);
			}
		};

	public:
		/// Asserts that multiple transactions can be added and removed.
		static void AssertCanAddAndRemoveMultipleTransactions() {
			// Arrange:
			AddRemoveInfos addRemoveInfos(4, 3);
			TestContext<MockAddRemoveCacheModifier> context(addRemoveInfos.RemovedInfosMap);

			// Act: add and remove via modifier, which flushes when destroyed
			{
				auto modifier = context.aggregate().modifier();
				addRemoveInfos.addRemove(modifier);
			}

			// Assert: check subscriber
			ASSERT_EQ(4u, context.subscriber().addedInfos().size());
			test::AssertEquivalent(addRemoveInfos.AddedInfos, context.subscriber().addedInfos(), "added");

			ASSERT_EQ(3u, context.subscriber().removedInfos().size());
			test::AssertEquivalent(addRemoveInfos.RemovedInfos, context.subscriber().removedInfos(), "removed");

			ASSERT_EQ(1u, context.subscriber().flushInfos().size());
			EXPECT_EQ(TTraits::CreateFlushInfo(4, 3), context.subscriber().flushInfos()[0]);
		}

		/// Asserts that a transaction can be removed and then added.
		static void AssertCanAddRemovedTransaction() {
			// Arrange:
			AddRemoveInfos addRemoveInfos(4, 3);
			TestContext<MockAddRemoveCacheModifier> context(addRemoveInfos.RemovedInfosMap);

			// Act: add and remove via modifier, which flushes when destroyed
			{
				auto modifier = context.aggregate().modifier();
				addRemoveInfos.addRemove(modifier);

				// - add a transaction that was removed
				modifier.add(addRemoveInfos.RemovedInfos[1]);
			}

			// Assert: check subscriber
			ASSERT_EQ(4u, context.subscriber().addedInfos().size());
			test::AssertEquivalent(addRemoveInfos.AddedInfos, context.subscriber().addedInfos(), "added");

			ASSERT_EQ(2u, context.subscriber().removedInfos().size());
			addRemoveInfos.RemovedInfos.erase(addRemoveInfos.RemovedInfos.begin() + 1); // remove from the original before checking
			test::AssertEquivalent(addRemoveInfos.RemovedInfos, context.subscriber().removedInfos(), "removed");

			ASSERT_EQ(1u, context.subscriber().flushInfos().size());
			EXPECT_EQ(TTraits::CreateFlushInfo(4, 2), context.subscriber().flushInfos()[0]);
		}

		/// Asserts that a transaction can be added and then removed.
		static void AssertCanRemoveAddedTransaction() {
			// Arrange:
			AddRemoveInfos addRemoveInfos(4, 3);
			TestContext<MockAddRemoveCacheModifier> context(addRemoveInfos.RemovedInfosMap);

			// Act: add and remove via modifier, which flushes when destroyed
			{
				auto modifier = context.aggregate().modifier();
				addRemoveInfos.addRemove(modifier);

				// - remove a transaction that was added
				modifier.remove(addRemoveInfos.AddedInfos[2].EntityHash);
			}

			// Assert: check subscriber
			ASSERT_EQ(3u, context.subscriber().addedInfos().size());
			addRemoveInfos.AddedInfos.erase(addRemoveInfos.AddedInfos.begin() + 2); // remove from the original before checking
			test::AssertEquivalent(addRemoveInfos.AddedInfos, context.subscriber().addedInfos(), "added");

			ASSERT_EQ(3u, context.subscriber().removedInfos().size());
			test::AssertEquivalent(addRemoveInfos.RemovedInfos, context.subscriber().removedInfos(), "removed");

			ASSERT_EQ(1u, context.subscriber().flushInfos().size());
			EXPECT_EQ(TTraits::CreateFlushInfo(3, 3), context.subscriber().flushInfos()[0]);
		}

		// endregion

		// region aggregate transaction cache - exceptions

	private:
		class FlushOnlyChangeSubscriberType : public TTraits::CacheTraitsType::UnsupportedChangeSubscriberType {
		public:
			void flush() override {
				// do nothing
			}
		};

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wused-but-marked-unused"
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

	private:
		// custom subscriber with some unimplemented methods is expected
		template<typename TSubscriber = FlushOnlyChangeSubscriberType, typename TAction>
		static void RunExceptionTest(TAction action) {
			// Arrange:
			using CacheTraits = typename TTraits::CacheTraitsType;

			AddRemoveInfos addRemoveInfos(4, 3);
			typename CacheTraits::template MockCacheType<MockAddRemoveCacheModifier> cache(addRemoveInfos.RemovedInfosMap);
			auto pAggregate = CacheTraits::CreateAggregateCache(cache, std::make_unique<TSubscriber>());

			// Act:
			ASSERT_DEATH({
				auto modifier = pAggregate->modifier();
				action(modifier, addRemoveInfos);
			}, "");
		}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

	public:
		/// Asserts that an add exception crashes the process.
		static void AssertAddExceptionTerminatesProcess() {
			// Assert:
			RunExceptionTest<FlushOnlyChangeSubscriberType>([](auto& modifier, const auto&) {
				modifier.add(test::CreateRandomTransactionInfo());
			});
		}

		/// Asserts that a remove exception crashes the process.
		static void AssertRemoveExceptionTerminatesProcess() {
			// Assert:
			RunExceptionTest<FlushOnlyChangeSubscriberType>([](auto& modifier, const auto& addRemoveInfos) {
				modifier.remove(addRemoveInfos.RemovedInfosMap.cbegin()->first);
			});
		}

		/// Asserts that a flush exception crashes the process.
		static void AssertFlushExceptionTerminatesProcess() {
			// Assert:
			RunExceptionTest<typename TTraits::CacheTraitsType::UnsupportedChangeSubscriberType>([](const auto&, const auto&) {
				// do nothing, flush will still be called
			});
		}

		// endregion
	};

#define MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::BasicAggregateTransactionsCacheTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_AGGREGATE_TRANSACTIONS_CACHE_TESTS(TEST_CLASS, TRAITS_NAME) \
	MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, SizeDelegatesToCache) \
	MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, MemorySizeDelegatesToCache) \
	\
	MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, AddDelegatesToCacheAndSubscriberAfterSuccessfulCacheAdd) \
	MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, SingleNotifificationForRedundantAdds) \
	MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, AddDelegatesToOnlyCacheAfterFailedCacheAdd) \
	\
	MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, RemoveDelegatesToCacheAndSubscriberAfterSuccessfulCacheRemove) \
	MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, SingleNotifificationForRedundantRemoves) \
	MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, RemoveDelegatesToOnlyCacheAfterFailedCacheRemove) \
	\
	MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, ModifierDestructorDelegatesToFlush) \
	\
	MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, CanAddAndRemoveMultipleTransactions) \
	MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, CanAddRemovedTransaction) \
	MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, CanRemoveAddedTransaction) \
	\
	MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, AddExceptionTerminatesProcess) \
	MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, RemoveExceptionTerminatesProcess) \
	MAKE_AGGREGATE_TRANSACTIONS_CACHE_TEST(TEST_CLASS, TRAITS_NAME, FlushExceptionTerminatesProcess)
}}
