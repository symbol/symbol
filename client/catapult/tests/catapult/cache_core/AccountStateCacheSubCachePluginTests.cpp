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

#include "catapult/cache_core/AccountStateCacheSubCachePlugin.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/cache/SummaryAwareCacheStoragePluginTests.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AccountStateCacheSubCachePluginTests

	// region AccountStateCacheSummaryCacheStorage - saveAll / saveSummary

	namespace {
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);

		auto CreateConfigurationFromOptions(const AccountStateCacheTypes::Options& options) {
			auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
			blockChainConfig.MinHarvesterBalance = options.MinHarvesterBalance;
			blockChainConfig.HarvestingMosaicId = options.HarvestingMosaicId;
			return blockChainConfig;
		}

		std::vector<Address> AddAccountsWithBalances(AccountStateCacheDelta& delta, const std::vector<Amount>& balances) {
			auto addresses = test::GenerateRandomDataVector<Address>(balances.size());
			for (auto i = 0u; i < balances.size(); ++i) {
				delta.addAccount(addresses[i], Height(1));
				delta.find(addresses[i]).get().Balances.credit(Harvesting_Mosaic_Id, balances[i]);
			}

			return addresses;
		}

		template<typename TAction>
		void RunCacheStorageTest(Amount minHarvesterBalance, TAction action) {
			// Arrange:
			AccountStateCacheTypes::Options options;
			options.MinHarvesterBalance = minHarvesterBalance;
			options.HarvestingMosaicId = Harvesting_Mosaic_Id;

			test::TempDirectoryGuard dbDirGuard;
			auto config = CacheConfiguration(dbDirGuard.name(), utils::FileSize(), cache::PatriciaTreeStorageMode::Disabled);
			AccountStateCacheSubCachePlugin plugin(config, options);
			auto pStorage = plugin.createStorage();

			// Act + Assert:
			action(*pStorage, CreateConfigurationFromOptions(options), plugin.cache());
		}

		template<typename TAction>
		void RunSummarySaveTest(Amount minHarvesterBalance, size_t numExpectedAccounts, TAction checkAddresses) {
			// Arrange:
			RunCacheStorageTest(minHarvesterBalance, [numExpectedAccounts, checkAddresses](
					const auto& storage,
					const auto& blockChainConfig,
					const auto&) {
				auto catapultCache = test::CoreSystemCacheFactory::Create(blockChainConfig);
				auto cacheDelta = catapultCache.createDelta();
				auto& delta = cacheDelta.template sub<AccountStateCache>();
				auto balances = { Amount(1'000'000), Amount(500'000), Amount(750'000), Amount(1'250'000) };
				auto addresses = AddAccountsWithBalances(delta, balances);
				delta.updateHighValueAccounts(Height(1));

				std::vector<uint8_t> buffer;
				mocks::MockMemoryStream stream(buffer);

				// Act:
				storage.saveSummary(cacheDelta, stream);

				// Assert: all addresses were saved
				ASSERT_EQ(sizeof(uint64_t) + numExpectedAccounts * sizeof(Address), buffer.size());

				auto numAddresses = reinterpret_cast<uint64_t&>(buffer[0]);
				EXPECT_EQ(numExpectedAccounts, numAddresses);

				model::AddressSet savedAddresses;
				for (auto i = 0u; i < numAddresses; ++i)
					savedAddresses.insert(reinterpret_cast<Address&>(buffer[sizeof(uint64_t) + i * sizeof(Address)]));

				checkAddresses(addresses, savedAddresses);

				// - there was a single flush
				EXPECT_EQ(1u, stream.numFlushes());
			});
		}
	}

	TEST(TEST_CLASS, CannotSaveAll) {
		// Arrange:
		RunCacheStorageTest(Amount(2'000'000), [](const auto& storage, const auto& blockChainConfig, const auto&) {
			auto catapultCache = test::CoreSystemCacheFactory::Create(blockChainConfig);
			auto cacheView = catapultCache.createView();

			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);

			// Act + Assert:
			EXPECT_THROW(storage.saveAll(cacheView, stream), catapult_invalid_argument);
		});
	}

	TEST(TEST_CLASS, CanSaveSummaryWithZeroHighValueAddresses) {
		// Act:
		RunSummarySaveTest(Amount(2'000'000), 0, [](const auto&, const auto& savedAddresses) {
			// Assert:
			EXPECT_TRUE(savedAddresses.empty());
		});
	}

	TEST(TEST_CLASS, CanSaveSummaryWithSingleHighValueAddress) {
		// Act:
		RunSummarySaveTest(Amount(1'111'111), 1, [](const auto& originalAddresses, const auto& savedAddresses) {
			// Assert:
			EXPECT_EQ(model::AddressSet{ originalAddresses[3] }, savedAddresses);
		});
	}

	TEST(TEST_CLASS, CanSaveSummaryWithMultipleHighValueAddresses) {
		// Act:
		RunSummarySaveTest(Amount(700'000), 3, [](const auto& originalAddresses, const auto& savedAddresses) {
			// Assert:
			EXPECT_EQ((model::AddressSet{ originalAddresses[0], originalAddresses[2], originalAddresses[3] }), savedAddresses);
		});
	}

	// endregion

	// region AccountStateCacheSummaryCacheStorage - loadAll

	namespace {
		void RunSummaryLoadTest(size_t numAccounts) {
			// Arrange:
			test::TempDirectoryGuard dbDirGuard;
			auto config = CacheConfiguration(dbDirGuard.name(), utils::FileSize(), cache::PatriciaTreeStorageMode::Disabled);
			AccountStateCacheSubCachePlugin plugin(config, AccountStateCacheTypes::Options());
			auto pStorage = plugin.createStorage();

			auto addresses = test::GenerateRandomDataVector<Address>(numAccounts);

			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);
			io::Write64(stream, numAccounts);
			stream.write({ reinterpret_cast<const uint8_t*>(addresses.data()), numAccounts * sizeof(Address) });

			// Act:
			pStorage->loadAll(stream, 0);

			// Assert: all addresses were loaded
			auto view = plugin.cache().createView();
			const auto& highValueAddresses = view->highValueAccounts().addresses();
			EXPECT_EQ(numAccounts, highValueAddresses.size());
			EXPECT_EQ(model::AddressSet(addresses.cbegin(), addresses.cend()), highValueAddresses);
		}
	}

	TEST(TEST_CLASS, CanLoadSummaryZeroHighValueAddresses) {
		RunSummaryLoadTest(0);
	}

	TEST(TEST_CLASS, CanLoadSummarySingleHighValueAddress) {
		RunSummaryLoadTest(1);
	}

	TEST(TEST_CLASS, CanLoadSummaryMultipleHighValueAddresses) {
		RunSummaryLoadTest(3);
	}

	// endregion

	// region AccountStateCacheSubCachePlugin

	namespace {
		struct PluginTraits {
			static constexpr auto Base_Name = "AccountStateCache";

			class PluginType : public AccountStateCacheSubCachePlugin {
			public:
				explicit PluginType(const CacheConfiguration& config)
						: AccountStateCacheSubCachePlugin(config, AccountStateCacheTypes::Options())
				{}
			};
		};
	}

	DEFINE_SUMMARY_AWARE_CACHE_STORAGE_PLUGIN_TESTS(PluginTraits)

	TEST(TEST_CLASS, CanRoundtripHighValueAddresses_Full) {
		// Arrange:
		AccountStateCacheTypes::Options options;
		options.MinHarvesterBalance = Amount(1'000'000);
		options.HarvestingMosaicId = Harvesting_Mosaic_Id;

		auto config = CacheConfiguration();

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);

		// Act: save 2/4 high value accounts
		std::vector<Address> addresses;
		{
			auto catapultCache = test::CoreSystemCacheFactory::Create(CreateConfigurationFromOptions(options));
			{
				auto cacheDelta = catapultCache.createDelta();
				auto& delta = cacheDelta.sub<AccountStateCache>();
				addresses = AddAccountsWithBalances(delta, { Amount(1'000'000), Amount(500'000), Amount(750'000), Amount(1'250'000) });
				delta.updateHighValueAccounts(Height(1));
				catapultCache.commit(Height(1));
			}

			AccountStateCacheSubCachePlugin plugin(config, options);
			auto pStorage = plugin.createStorage();

			auto cacheView = catapultCache.createView();
			pStorage->saveAll(cacheView, stream);

			// Sanity:
			const auto& view = cacheView.sub<AccountStateCache>();
			EXPECT_EQ(4u, view.size());
			EXPECT_EQ(2u, view.highValueAccounts().addresses().size());
		}

		// - reload to roundtrip
		AccountStateCacheSubCachePlugin plugin(config, options);
		auto pStorage = plugin.createStorage();
		pStorage->loadAll(stream, 1);

		// Assert: all accounts were loaded
		auto& view = *plugin.cache().createView();
		EXPECT_EQ(4u, view.size());

		// - all high value addresses were loaded
		const auto& highValueAddresses = view.highValueAccounts().addresses();
		EXPECT_EQ(2u, highValueAddresses.size());
		EXPECT_EQ(model::AddressSet({ addresses[0], addresses[3] }), highValueAddresses);
	}

	// endregion
}}
