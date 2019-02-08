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
#include "tests/test/cache/SummaryAwareCacheStoragePluginTests.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AccountStateCacheSubCachePluginTests

	// region AccountStateCacheSummaryCacheStorage - saveAll

	namespace {
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);

		std::vector<Address> AddAccountsWithBalances(AccountStateCache& cache, const std::vector<Amount>& balances) {
			auto delta = cache.createDelta();

			auto addresses = test::GenerateRandomDataVector<Address>(balances.size());
			for (auto i = 0u; i < balances.size(); ++i) {
				delta->addAccount(addresses[i], Height(1));
				delta->find(addresses[i]).get().Balances.credit(Harvesting_Mosaic_Id, balances[i]);
			}

			cache.commit();
			return addresses;
		}

		template<typename TAction>
		void RunSummarySaveTest(Amount minHighValueAccountBalance, size_t numExpectedAccounts, TAction checkAddresses) {
			// Arrange:
			AccountStateCacheTypes::Options options;
			options.MinHighValueAccountBalance = minHighValueAccountBalance;
			options.HarvestingMosaicId = Harvesting_Mosaic_Id;
			AccountStateCache cache(CacheConfiguration(), options);
			AccountStateCacheSummaryCacheStorage storage(cache);

			auto balances = { Amount(1'000'000), Amount(500'000), Amount(750'000), Amount(1'250'000) };
			auto addresses = AddAccountsWithBalances(cache, balances);

			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream("", buffer);

			// Act:
			storage.saveAll(stream);

			// Assert: all addresses were saved
			ASSERT_EQ(sizeof(uint64_t) + numExpectedAccounts * sizeof(Address), buffer.size());

			auto numAddresses = reinterpret_cast<uint64_t&>(*buffer.data());
			EXPECT_EQ(numExpectedAccounts, numAddresses);

			model::AddressSet savedAddresses;
			for (auto i = 0u; i < numAddresses; ++i)
				savedAddresses.insert(reinterpret_cast<Address&>(*(buffer.data() + sizeof(uint64_t) + i * sizeof(Address))));

			checkAddresses(addresses, savedAddresses);

			// - there was a single flush
			EXPECT_EQ(1u, stream.numFlushes());
		}
	}

	TEST(TEST_CLASS, CanSaveZeroHighValueAddresses) {
		// Act:
		RunSummarySaveTest(Amount(2'000'000), 0, [](const auto&, const auto& savedAddresses) {
			// Assert:
			EXPECT_TRUE(savedAddresses.empty());
		});
	}

	TEST(TEST_CLASS, CanSaveSingleHighValueAddress) {
		// Act:
		RunSummarySaveTest(Amount(1'111'111), 1, [](const auto& originalAddresses, const auto& savedAddresses) {
			// Assert:
			EXPECT_EQ(model::AddressSet{ originalAddresses[3] }, savedAddresses);
		});
	}

	TEST(TEST_CLASS, CanSaveMultipleHighValueAddresses) {
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
			auto config = CacheConfiguration();
			AccountStateCache cache(config, AccountStateCacheTypes::Options());
			AccountStateCacheSummaryCacheStorage storage(cache);

			auto addresses = test::GenerateRandomDataVector<Address>(numAccounts);

			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream("", buffer);
			io::Write64(stream, numAccounts);
			stream.write({ reinterpret_cast<const uint8_t*>(addresses.data()), numAccounts * sizeof(Address) });

			// Act:
			storage.loadAll(stream, 0);

			// Assert: all addresses were saved
			auto view = cache.createView();
			EXPECT_EQ(numAccounts, view->highValueAddresses().size());
			EXPECT_EQ(model::AddressSet(addresses.cbegin(), addresses.cend()), view->highValueAddresses());
		}
	}

	TEST(TEST_CLASS, CanLoadZeroHighValueAddresses) {
		// Assert:
		RunSummaryLoadTest(0);
	}

	TEST(TEST_CLASS, CanLoadSingleHighValueAddress) {
		// Assert:
		RunSummaryLoadTest(1);
	}

	TEST(TEST_CLASS, CanLoadMultipleHighValueAddresses) {
		// Assert:
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

	// endregion
}}
