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

#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_tx/MemoryUtCache.h"
#include "catapult/chain/UtUpdater.h"
#include "catapult/extensions/ExecutionConfigurationFactory.h"
#include "catapult/thread/ThreadGroup.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/RealTransactionFactory.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS UtUpdaterIntegrityTests

	namespace {
		constexpr auto Default_Time = Timestamp(987);

		uint64_t GetNumIterations() {
			return test::GetStressIterationCount() ? 5'000 : 250;
		}

		// region UpdaterTestContext

		class UpdaterTestContext {
		public:
			UpdaterTestContext()
					: m_pPluginManager(CreatePluginManager())
					, m_transactionsCache(cache::MemoryCacheOptions(
							utils::FileSize::FromKilobytes(1),
							utils::FileSize::FromBytes(test::GetTransferTransactionSize() * GetNumIterations() * 2)))
					, m_cache(CreateCatapultCache())
					, m_updater(
							m_transactionsCache,
							m_cache,
							BlockFeeMultiplier(0),
							extensions::CreateExecutionConfiguration(*m_pPluginManager),
							[]() { return Default_Time; },
							[](const auto&, const auto&, auto) {},
							[](const auto&, const auto&) { return false; })
			{}

		public:
			cache::MemoryUtCache& transactionsCache() {
				return m_transactionsCache;
			}

			cache::CatapultCache& cache() {
				return m_cache;
			}

			UtUpdater& updater() {
				return m_updater;
			}

		private:
			static std::shared_ptr<plugins::PluginManager> CreatePluginManager() {
				auto config = test::CreatePrototypicalBlockChainConfiguration();
				config.Plugins.emplace("catapult.plugins.transfer", utils::ConfigurationBag({{ "", { { "maxMessageSize", "0" } } }}));
				return test::CreatePluginManagerWithRealPlugins(config);
			}

			static cache::CatapultCache CreateCatapultCache() {
				auto cache = test::CoreSystemCacheFactory::Create(test::CreatePrototypicalBlockChainConfiguration());
				test::AddMarkerAccount(cache);
				return cache;
			}

		private:
			std::shared_ptr<plugins::PluginManager> m_pPluginManager;
			cache::MemoryUtCache m_transactionsCache;
			cache::CatapultCache m_cache;
			UtUpdater m_updater;
		};

		// endregion
	}

	NO_STRESS_TEST(TEST_CLASS, UtUpdaterUpdateOverloadsAreThreadSafe) {
		// Arrange:
		UpdaterTestContext context;

		// - seed an account with an initial balance of N
		auto senderKeyPair = test::GenerateKeyPair();
		{
			auto mosaicId = test::Default_Currency_Mosaic_Id;
			auto cacheDelta = context.cache().createDelta();
			auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();
			accountStateCacheDelta.addAccount(senderKeyPair.publicKey(), Height(1));
			accountStateCacheDelta.find(senderKeyPair.publicKey()).get().Balances.credit(mosaicId, Amount(GetNumIterations()));
			context.cache().commit(Height(1));
		}

		// Act:
		// - simulate tx dispatcher processing N elements of 1 tx transfering 1 unit each
		thread::ThreadGroup threads;
		threads.spawn([&senderKeyPair, &updater = context.updater()] {
			auto recipient = test::GenerateRandomByteArray<Key>();
			for (auto i = 0u; i < GetNumIterations(); ++i) {
				auto pTransaction = test::CreateTransferTransaction(senderKeyPair, recipient, Amount(1));
				pTransaction->MaxFee = Amount(0);
				pTransaction->Deadline = Default_Time + Timestamp(1);
				model::TransactionInfo transactionInfo(std::move(pTransaction), test::GenerateRandomByteArray<Hash256>());

				std::vector<model::TransactionInfo> transactionInfos;
				transactionInfos.emplace_back(std::move(transactionInfo));
				updater.update(transactionInfos);
			}
		});

		// - simulate block dispatcher processing N block elements with single confirmed tx
		threads.spawn([&updater = context.updater()] {
			auto hash = test::GenerateRandomByteArray<Hash256>();
			for (auto i = 0u; i < GetNumIterations(); ++i)
				updater.update({ &hash }, {});
		});

		// - wait for all threads
		threads.join();

		// Assert: all transactions are in the UT cache
		EXPECT_EQ(GetNumIterations(), context.transactionsCache().view().size());
	}
}}
