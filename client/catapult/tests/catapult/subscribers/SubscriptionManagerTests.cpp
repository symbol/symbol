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

#include "catapult/subscribers/SubscriptionManager.h"
#include "catapult/config/CatapultConfiguration.h"
#include "catapult/ionet/Node.h"
#include "catapult/model/ChainScore.h"
#include "tests/catapult/subscribers/test/UnsupportedSubscribers.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/other/MutableCatapultConfiguration.h"
#include "tests/test/other/mocks/MockBlockChangeSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS SubscriptionManagerTests

	using UnsupportedPtChangeSubscriber = test::UnsupportedPtChangeSubscriber<test::UnsupportedFlushBehavior::Ignore>;
	using UnsupportedUtChangeSubscriber = test::UnsupportedUtChangeSubscriber<test::UnsupportedFlushBehavior::Ignore>;

	namespace {
		config::CatapultConfiguration CreateConfiguration() {
			return test::MutableCatapultConfiguration().ToConst();
		}
	}

	// region traits

	namespace {
		struct BlockChangeTraits {
			using UnsupportedSubscriberType = test::UnsupportedBlockChangeSubscriber;

			static auto CreateAggregate(SubscriptionManager& manager) {
				return manager.createBlockChangeSubscriber();
			}

			static void AddSubscriber(SubscriptionManager& manager, std::unique_ptr<io::BlockChangeSubscriber>&& pSubscriber) {
				manager.addBlockChangeSubscriber(std::move(pSubscriber));
			}

			static void Notify(io::BlockChangeSubscriber& subscriber) {
				subscriber.notifyDropBlocksAfter(Height(11));
			}
		};

		struct PtChangeTraits {
			using UnsupportedSubscriberType = UnsupportedPtChangeSubscriber;

			static auto CreateAggregate(SubscriptionManager& manager) {
				return manager.createPtChangeSubscriber();
			}

			static void AddSubscriber(SubscriptionManager& manager, std::unique_ptr<cache::PtChangeSubscriber>&& pSubscriber) {
				manager.addPtChangeSubscriber(std::move(pSubscriber));
			}

			static void Notify(cache::PtChangeSubscriber& subscriber) {
				subscriber.notifyAddPartials({});
			}
		};

		struct UtChangeTraits {
			using UnsupportedSubscriberType = UnsupportedUtChangeSubscriber;

			static auto CreateAggregate(SubscriptionManager& manager) {
				return manager.createUtChangeSubscriber();
			}

			static void AddSubscriber(SubscriptionManager& manager, std::unique_ptr<cache::UtChangeSubscriber>&& pSubscriber) {
				manager.addUtChangeSubscriber(std::move(pSubscriber));
			}

			static void Notify(cache::UtChangeSubscriber& subscriber) {
				subscriber.notifyAdds({});
			}
		};

		struct FinalizationTraits {
			using UnsupportedSubscriberType = test::UnsupportedFinalizationSubscriber;

			static auto CreateAggregate(SubscriptionManager& manager) {
				return manager.createFinalizationSubscriber();
			}

			static void AddSubscriber(SubscriptionManager& manager, std::unique_ptr<FinalizationSubscriber>&& pSubscriber) {
				manager.addFinalizationSubscriber(std::move(pSubscriber));
			}

			static void Notify(FinalizationSubscriber& subscriber) {
				subscriber.notifyFinalizedBlock(Height(), Hash256(), FinalizationPoint());
			}
		};

		struct NodeTraits {
			using UnsupportedSubscriberType = test::UnsupportedNodeSubscriber;

			static auto CreateAggregate(SubscriptionManager& manager) {
				return manager.createNodeSubscriber();
			}

			static void AddSubscriber(SubscriptionManager& manager, std::unique_ptr<NodeSubscriber>&& pSubscriber) {
				manager.addNodeSubscriber(std::move(pSubscriber));
			}

			static void Notify(NodeSubscriber& subscriber) {
				subscriber.notifyNode(ionet::Node());
			}
		};

		struct StateChangeTraits {
			using UnsupportedSubscriberType = test::UnsupportedStateChangeSubscriber;

			static auto CreateAggregate(SubscriptionManager& manager) {
				return manager.createStateChangeSubscriber();
			}

			static void AddSubscriber(SubscriptionManager& manager, std::unique_ptr<StateChangeSubscriber>&& pSubscriber) {
				manager.addStateChangeSubscriber(std::move(pSubscriber));
			}

			static void Notify(StateChangeSubscriber& subscriber) {
				subscriber.notifyScoreChange(model::ChainScore());
			}
		};

		struct TransactionStatusTraits {
			using UnsupportedSubscriberType = test::UnsupportedTransactionStatusSubscriber<test::UnsupportedFlushBehavior::Ignore>;

			static auto CreateAggregate(SubscriptionManager& manager) {
				return manager.createTransactionStatusSubscriber();
			}

			static void AddSubscriber(SubscriptionManager& manager, std::unique_ptr<TransactionStatusSubscriber>&& pSubscriber) {
				manager.addTransactionStatusSubscriber(std::move(pSubscriber));
			}

			static void Notify(TransactionStatusSubscriber& subscriber) {
				subscriber.notifyStatus(*test::GenerateRandomTransaction(), test::GenerateRandomByteArray<Hash256>(), 123);
			}
		};

		struct BlockStorageTraits : public BlockChangeTraits {
			static auto CreateAggregate(SubscriptionManager& manager) {
				io::BlockChangeSubscriber* pAggregateSubscriber;
				return manager.createBlockStorage(pAggregateSubscriber);
			}
		};

		struct PtCacheTraits : public PtChangeTraits {
			static auto CreateAggregate(SubscriptionManager& manager) {
				return manager.createPtCache(cache::MemoryCacheOptions(100, 100));
			}
		};

		struct UtCacheTraits : public UtChangeTraits {
			static auto CreateAggregate(SubscriptionManager& manager) {
				return manager.createUtCache(cache::MemoryCacheOptions(100, 100));
			}
		};
	}

	// endregion

	// region constructor

	TEST(TEST_CLASS, CanCreateManager) {
		// Act:
		auto config = CreateConfiguration();
		SubscriptionManager manager(config);

		// Assert: file storage is valid (does not throw or crash)
		manager.fileStorage();
	}

	// endregion

	// region single aggregate creation

#define SINGLE_AGGREGATE_CREATION_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_BlockChange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockChangeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_PtChange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PtChangeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_UtChange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<UtChangeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Finalization) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<FinalizationTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Node) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NodeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_StateChange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<StateChangeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_TransactionStatus) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionStatusTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_BlockStorage) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockStorageTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_PtCache) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PtCacheTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_UtCache) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<UtCacheTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	SINGLE_AGGREGATE_CREATION_TEST(CannotCreateAggregateMultipleTimes) {
		// Arrange:
		auto config = CreateConfiguration();
		SubscriptionManager manager(config);

		// - create a subscriber
		TTraits::CreateAggregate(manager);

		// Act + Assert: cannot create another subscriber
		EXPECT_THROW(TTraits::CreateAggregate(manager), catapult_invalid_argument);
	}

	SINGLE_AGGREGATE_CREATION_TEST(CannotAddSubscriptionsAfterCreatingSubscriber) {
		// Arrange:
		auto config = CreateConfiguration();
		SubscriptionManager manager(config);

		// - create a subscriber
		TTraits::CreateAggregate(manager);

		// Act + Assert: cannot add new subscriptions
		auto pNewSubscriber = std::make_unique<typename TTraits::UnsupportedSubscriberType>();
		EXPECT_THROW(TTraits::AddSubscriber(manager, std::move(pNewSubscriber)), catapult_invalid_argument);
	}

	// endregion

	// region create - subscriber

#define BASIC_SUBSCRIPTION_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_BlockChange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockChangeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_PtChange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PtChangeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_UtChange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<UtChangeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Finalization) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<FinalizationTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Node) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NodeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_StateChange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<StateChangeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_TransactionStatus) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionStatusTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	BASIC_SUBSCRIPTION_TEST(CanCreateAggregateWithoutSubscriptions) {
		// Arrange:
		auto config = CreateConfiguration();
		SubscriptionManager manager(config);

		// Act:
		auto pSubscriber = TTraits::CreateAggregate(manager);

		// Assert: notification should not trigger subscriber exception
		ASSERT_TRUE(!!pSubscriber);
		TTraits::Notify(*pSubscriber);
	}

	BASIC_SUBSCRIPTION_TEST(CanCreateAggregateWithSubscriptions) {
		// Arrange:
		auto config = CreateConfiguration();
		SubscriptionManager manager(config);

		// Act:
		TTraits::AddSubscriber(manager, std::make_unique<typename TTraits::UnsupportedSubscriberType>());
		auto pSubscriber = TTraits::CreateAggregate(manager);

		// Assert: notification should trigger subscriber exception
		ASSERT_TRUE(!!pSubscriber);
		EXPECT_THROW(TTraits::Notify(*pSubscriber), catapult_runtime_error);
	}

	// endregion

	// region create - block storage

	TEST(TEST_CLASS, CanCreateBlockStorageWithoutSubscriptions) {
		// Arrange:
		auto config = CreateConfiguration();
		SubscriptionManager manager(config);
		const auto& fileStorage = manager.fileStorage();

		// Act:
		io::BlockChangeSubscriber* pAggregateSubscriber;
		auto pBlockStorage = manager.createBlockStorage(pAggregateSubscriber);

		// Assert: the file storage is returned as is
		ASSERT_TRUE(!!pBlockStorage);
		EXPECT_EQ(&fileStorage, pBlockStorage.get());

		// - no subscriber was returned
		EXPECT_FALSE(!!pAggregateSubscriber);
	}

	TEST(TEST_CLASS, CanCreateBlockStorageWithSubscriptions) {
		// Arrange:
		auto config = CreateConfiguration();
		SubscriptionManager manager(config);
		const auto& fileStorage = manager.fileStorage();

		auto pMockBlockChangeSubscriber = std::make_unique<mocks::MockBlockChangeSubscriber>();
		const auto& mockBlockChangeSubscriber = *pMockBlockChangeSubscriber;

		// Act:
		io::BlockChangeSubscriber* pAggregateSubscriber;
		manager.addBlockChangeSubscriber(std::move(pMockBlockChangeSubscriber));
		auto pBlockStorage = manager.createBlockStorage(pAggregateSubscriber);

		// Assert: the file storage is not returned directly
		ASSERT_TRUE(!!pBlockStorage);
		EXPECT_NE(&fileStorage, pBlockStorage.get());

		// - dropBlocksAfter should delegate
		pBlockStorage->dropBlocksAfter(Height(123));

		ASSERT_EQ(1u, mockBlockChangeSubscriber.dropBlocksAfterHeights().size());
		EXPECT_EQ(Height(123), mockBlockChangeSubscriber.dropBlocksAfterHeights()[0]);

		// - subscriber was returned and should delegate
		ASSERT_TRUE(!!pAggregateSubscriber);

		pAggregateSubscriber->notifyDropBlocksAfter(Height(256));

		ASSERT_EQ(2u, mockBlockChangeSubscriber.dropBlocksAfterHeights().size());
		EXPECT_EQ(Height(256), mockBlockChangeSubscriber.dropBlocksAfterHeights()[1]);
	}

	// endregion

	// region create - pt / ut caches

	namespace {
		struct PtTraits {
			static auto CreateCache(SubscriptionManager& manager) {
				return manager.createPtCache(cache::MemoryCacheOptions(100, 100));
			}

			static void AddSubscriberWithAddCounter(SubscriptionManager& manager, size_t& counter) {
				class PtChangeSubscriberWithAddCounter : public UnsupportedPtChangeSubscriber {
				public:
					explicit PtChangeSubscriberWithAddCounter(size_t& addCounter) : m_addCounter(addCounter)
					{}

				public:
					void notifyAddPartials(const TransactionInfos&) override {
						++m_addCounter;
					}

				private:
					size_t& m_addCounter;
				};

				manager.addPtChangeSubscriber(std::make_unique<PtChangeSubscriberWithAddCounter>(counter));
			}
		};

		struct UtTraits {
			static auto CreateCache(SubscriptionManager& manager) {
				return manager.createUtCache(cache::MemoryCacheOptions(100, 100));
			}

			static void AddSubscriberWithAddCounter(SubscriptionManager& manager, size_t& counter) {
				class UtChangeSubscriberWithAddCounter : public UnsupportedUtChangeSubscriber {
				public:
					explicit UtChangeSubscriberWithAddCounter(size_t& addCounter) : m_addCounter(addCounter)
					{}

				public:
					void notifyAdds(const TransactionInfos&) override {
						++m_addCounter;
					}

				private:
					size_t& m_addCounter;
				};

				manager.addUtChangeSubscriber(std::make_unique<UtChangeSubscriberWithAddCounter>(counter));
			}
		};
	}

#define CACHE_SUBSCRIPTION_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Pt) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PtTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Ut) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<UtTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	CACHE_SUBSCRIPTION_TEST(CanCreateCacheWithoutSubscriptions) {
		// Arrange:
		auto config = CreateConfiguration();
		SubscriptionManager manager(config);

		// Act:
		auto pCache = TTraits::CreateCache(manager);

		// Assert:
		ASSERT_TRUE(!!pCache);
	}

	CACHE_SUBSCRIPTION_TEST(CanCreateCacheWithSubscriptions) {
		// Arrange:
		auto config = CreateConfiguration();
		SubscriptionManager manager(config);
		size_t counter = 0u;

		// Act:
		TTraits::AddSubscriberWithAddCounter(manager, counter);
		auto pCache = TTraits::CreateCache(manager);

		// Assert:
		ASSERT_TRUE(!!pCache);

		// - add should increment the counter
		pCache->modifier().add(test::CreateRandomTransactionInfo());

		EXPECT_EQ(1u, counter);
	}

	// endregion
}}
