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

#include "partialtransaction/src/PtBootstrapperService.h"
#include "catapult/cache_tx/MemoryPtCache.h"
#include "catapult/extensions/Results.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace partialtransaction {

#define TEST_CLASS PtBootstrapperServiceTests

	// region PtServerHooks

	namespace {
		struct CosignedTransactionInfosConsumerTraits {
			using ParamType = CosignedTransactionInfos;

			static auto Get(const PtServerHooks& hooks) {
				return hooks.cosignedTransactionInfosConsumer();
			}

			static void Set(PtServerHooks& hooks, const CosignedTransactionInfosConsumer& consumer) {
				hooks.setCosignedTransactionInfosConsumer(consumer);
			}
		};

		struct PtRangeConsumerTraits {
			using ParamType = model::AnnotatedEntityRange<model::Transaction>;

			static auto Get(const PtServerHooks& hooks) {
				return hooks.ptRangeConsumer();
			}

			static void Set(PtServerHooks& hooks, const handlers::TransactionRangeHandler& consumer) {
				hooks.setPtRangeConsumer(consumer);
			}
		};

		struct CosignatureRangeConsumerTraits {
			using ParamType = model::AnnotatedEntityRange<model::DetachedCosignature>;

			static auto Get(const PtServerHooks& hooks) {
				return hooks.cosignatureRangeConsumer();
			}

			static void Set(PtServerHooks& hooks, const handlers::RangeHandler<model::DetachedCosignature>& consumer) {
				hooks.setCosignatureRangeConsumer(consumer);
			}
		};
	}

#define CONSUMER_HOOK_TEST_ENTRY(TEST_NAME, CONSUMER_FACTORY_NAME) \
	TEST(TEST_CLASS, Hooks_##TEST_NAME##_##CONSUMER_FACTORY_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CONSUMER_FACTORY_NAME##Traits>(); \
	}

#define CONSUMER_HOOK_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	CONSUMER_HOOK_TEST_ENTRY(TEST_NAME, CosignedTransactionInfosConsumer) \
	CONSUMER_HOOK_TEST_ENTRY(TEST_NAME, PtRangeConsumer) \
	CONSUMER_HOOK_TEST_ENTRY(TEST_NAME, CosignatureRangeConsumer) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	CONSUMER_HOOK_TEST(CannotAccessWhenUnset) {
		// Arrange:
		PtServerHooks hooks;

		// Act + Assert:
		EXPECT_THROW(TTraits::Get(hooks), catapult_invalid_argument);
	}

	CONSUMER_HOOK_TEST(CanSetOnce) {
		// Arrange:
		PtServerHooks hooks;

		typename TTraits::ParamType seedParam;
		const auto* pSeedParam = &seedParam;
		std::vector<decltype(pSeedParam)> consumedParams;

		TTraits::Set(hooks, [&consumedParams](auto&& param) {
			consumedParams.push_back(&param);
		});

		// Act:
		auto factory = TTraits::Get(hooks);
		ASSERT_TRUE(!!factory);

		factory(std::move(seedParam));

		// Assert: the param created above should be passed (and moved) down
		ASSERT_EQ(1u, consumedParams.size());
		EXPECT_EQ(pSeedParam, consumedParams[0]);
	}

	CONSUMER_HOOK_TEST(CannotSetMultipleTimes) {
		// Arrange:
		PtServerHooks hooks;
		TTraits::Set(hooks, [](auto&&) {});

		// Act + Assert:
		EXPECT_THROW(TTraits::Set(hooks, [](auto&&) {}), catapult_invalid_argument);
	}

	// endregion

	namespace {
		struct PtBootstrapperServiceTraits {
			static auto CreateRegistrar() {
				return CreatePtBootstrapperServiceRegistrar([]() {
					auto cacheOptions = cache::MemoryCacheOptions(utils::FileSize(), utils::FileSize::FromKilobytes(2));
					return std::make_unique<cache::MemoryPtCacheProxy>(cacheOptions);
				});
			}
		};

		using TestContext = test::ServiceLocatorTestContext<PtBootstrapperServiceTraits>;
	}

	// region PtBootstrapperService basic

	ADD_SERVICE_REGISTRAR_INFO_TEST(PtBootstrapper, Initial)

	TEST(TEST_CLASS, PtCacheServiceIsRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(2u, context.locator().numServices());
		EXPECT_EQ(2u, context.locator().counters().size());

		// - service
		const auto& ptCache = GetMemoryPtCache(context.locator());
		EXPECT_EQ(0u, ptCache.view().size());

		// - counter
		EXPECT_EQ(0u, context.counter("PT CACHE"));
		EXPECT_EQ(0u, context.counter("PT CACHE MEM"));
	}

	TEST(TEST_CLASS, PtHooksServiceIsRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(2u, context.locator().numServices());

		// - service (get does not throw)
		GetPtServerHooks(context.locator());
	}

	// endregion

	// region PtBootstrapperService hooks (TransactionsChangeHandler)

	TEST(TEST_CLASS, TransactionsChangeHandlerRemovesConfirmedTransactionsFromCache) {
		// Arrange:
		TestContext context;
		context.boot();

		// - seed the cache (deadlines t[+1]..t[+6])
		auto transactionInfos = test::CreateTransactionInfos(6, [](auto i) {
			return test::CreateDefaultNetworkTimeSupplier()() + utils::TimeSpan::FromHours(i + 1);
		});
		auto& ptCache = GetMemoryPtCache(context.locator());
		for (const auto& transactionInfo : transactionInfos)
			ptCache.modifier().add(transactionInfo);

		// Act: trigger deletions of three infos
		auto handler = context.testState().state().hooks().transactionsChangeHandler();
		utils::HashPointerSet addedTransactionHashes{
				&transactionInfos[1].EntityHash,
				&transactionInfos[2].EntityHash,
				&transactionInfos[4].EntityHash
		};
		std::vector<model::TransactionInfo> revertedTransactionInfos;
		handler(consumers::TransactionsChangeInfo(addedTransactionHashes, revertedTransactionInfos));

		// Assert:
		auto view = ptCache.view();
		EXPECT_EQ(3u, view.size());
		EXPECT_TRUE(view.find(transactionInfos[0].EntityHash));
		EXPECT_TRUE(view.find(transactionInfos[3].EntityHash));
		EXPECT_TRUE(view.find(transactionInfos[5].EntityHash));

		// - no status events were raised
		const auto& subscriber = context.testState().transactionStatusSubscriber();
		EXPECT_EQ(0u, subscriber.params().size());
	}

	TEST(TEST_CLASS, TransactionsChangeHandlerPurgesExpiredTransactionsFromCache) {
		// Arrange:
		TestContext context;
		context.boot();

		// - seed the cache (deadlines t[+1.5]..t[-3.5])
		auto transactionInfos = test::CreateTransactionInfos(6, [](auto i) {
			return SubtractNonNegative(
					test::CreateDefaultNetworkTimeSupplier()(),
					utils::TimeSpan::FromHours(i)) + utils::TimeSpan::FromMinutes(90);
		});
		auto& ptCache = GetMemoryPtCache(context.locator());
		for (const auto& transactionInfo : transactionInfos)
			ptCache.modifier().add(transactionInfo);

		// Act: trigger pruning of four infos (t[+1.5] and t[+0.5] are preserved)
		auto handler = context.testState().state().hooks().transactionsChangeHandler();
		utils::HashPointerSet addedTransactionHashes;
		std::vector<model::TransactionInfo> revertedTransactionInfos;
		handler(consumers::TransactionsChangeInfo(addedTransactionHashes, revertedTransactionInfos));

		// Assert:
		auto view = ptCache.view();
		EXPECT_EQ(2u, view.size());
		EXPECT_TRUE(view.find(transactionInfos[0].EntityHash));
		EXPECT_TRUE(view.find(transactionInfos[1].EntityHash));

		// - check status notifications
		const auto& subscriber = context.testState().transactionStatusSubscriber();
		ASSERT_EQ(4u, subscriber.params().size());
		for (auto i = 0u; i < subscriber.params().size(); ++i) {
			auto message = "status at " + std::to_string(i);
			const auto& info = transactionInfos[5 - i]; // notice that infos are ordered from largest to smallest deadline
			const auto& status = subscriber.params()[i];

			// - compare a copy of the hash because the original (pruned) info is destroyed
			EXPECT_EQ(*info.pEntity, status.Transaction) << message;
			EXPECT_EQ(info.EntityHash, status.HashCopy) << message;
			EXPECT_EQ(utils::to_underlying_type(extensions::Failure_Extension_Partial_Transaction_Cache_Prune), status.Status) << message;
		}
	}

	// endregion

	// region PtBootstrapperService hooks (TransactionEventHandler)

	namespace {
		template<typename TInvokeHandler>
		void AssertTransactionEventHandlerIsNoOp(TInvokeHandler invokeHandler) {
			// Arrange:
			TestContext context;
			context.boot();

			// - seed the cache
			auto transactionInfos = test::CreateTransactionInfos(3);
			auto& ptCache = GetMemoryPtCache(context.locator());
			for (const auto& transactionInfo : transactionInfos)
				ptCache.modifier().add(transactionInfo);

			// Act: invoke the handler and pass a matching entity hash to invokeHandler
			auto handler = context.testState().state().hooks().transactionEventHandler();
			invokeHandler(handler, transactionInfos[1].EntityHash);

			// Assert:
			auto view = ptCache.view();
			EXPECT_EQ(3u, view.size());
			for (auto i = 0u; i < transactionInfos.size(); ++i)
				EXPECT_TRUE(view.find(transactionInfos[i].EntityHash)) << "info at " << i;

			// - no status events were raised
			const auto& subscriber = context.testState().transactionStatusSubscriber();
			EXPECT_EQ(0u, subscriber.params().size());
		}
	}

	TEST(TEST_CLASS, TransactionEventHandlerIgnoresNonMatchingEventHashes) {
		// Act:
		AssertTransactionEventHandlerIsNoOp([](const auto& handler, const auto& matchingEntityHash) {
			// Act: call the handler with a mismatched event
			handler({ matchingEntityHash, static_cast<extensions::TransactionEvent>(0xFE) });
		});
	}

	TEST(TEST_CLASS, TransactionEventHandlerIgnoresMatchingEventHashesNotInCache) {
		// Act:
		AssertTransactionEventHandlerIsNoOp([](const auto& handler, const auto&) {
			// Act: call the handler with a hash not in the cache
			handler({ test::GenerateRandomByteArray<Hash256>(), extensions::TransactionEvent::Dependency_Removed });
		});
	}

	TEST(TEST_CLASS, TransactionEventHandlerRemovesMatchingEventHashesFromCache) {
		// Arrange:
		TestContext context;
		context.boot();

		// - seed the cache
		auto transactionInfos = test::CreateTransactionInfos(5);
		auto& ptCache = GetMemoryPtCache(context.locator());
		for (const auto& transactionInfo : transactionInfos)
			ptCache.modifier().add(transactionInfo);

		// Act: trigger removal of two transactions
		auto handler = context.testState().state().hooks().transactionEventHandler();
		handler({ transactionInfos[1].EntityHash, extensions::TransactionEvent::Dependency_Removed });
		handler({ transactionInfos[4].EntityHash, static_cast<extensions::TransactionEvent>(0xFF) });

		// Assert:
		auto view = ptCache.view();
		EXPECT_EQ(3u, view.size());
		EXPECT_TRUE(view.find(transactionInfos[0].EntityHash));
		EXPECT_TRUE(view.find(transactionInfos[2].EntityHash));
		EXPECT_TRUE(view.find(transactionInfos[3].EntityHash));

		// - check status notifications
		const auto& subscriber = context.testState().transactionStatusSubscriber();
		ASSERT_EQ(2u, subscriber.params().size());
		for (auto i = 0u; i < subscriber.params().size(); ++i) {
			auto message = "status at " + std::to_string(i);
			const auto& info = transactionInfos[0 == i ? 1 : 4];
			const auto& status = subscriber.params()[i];
			auto expectedStatus = utils::to_underlying_type(extensions::Failure_Extension_Partial_Transaction_Dependency_Removed);

			// - compare a copy of the hash because the original (removed) info is destroyed
			EXPECT_EQ(*info.pEntity, status.Transaction) << message;
			EXPECT_EQ(info.EntityHash, status.HashCopy) << message;
			EXPECT_EQ(expectedStatus, status.Status) << message;
		}
	}

	// endregion
}}
