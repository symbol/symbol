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
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/cache/SummaryAwareCacheStoragePluginTests.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AccountStateCacheSubCachePluginTests

	// region test utils

	namespace {
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);

		auto CreateConfigurationFromOptions(const AccountStateCacheTypes::Options& options) {
			auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
			blockChainConfig.MinHarvesterBalance = options.MinHarvesterBalance;
			blockChainConfig.MinVoterBalance = options.MinVoterBalance;
			blockChainConfig.HarvestingMosaicId = options.HarvestingMosaicId;
			return blockChainConfig;
		}

		std::vector<Address> AddAccountsWithBalances(
				AccountStateCacheDelta& delta,
				const std::vector<Amount>& balances,
				const std::vector<Key>& vrfPublicKeys,
				const std::vector<model::PinnedVotingKey>& votingPublicKeys) {
			auto addresses = test::GenerateRandomDataVector<Address>(balances.size());
			for (auto i = 0u; i < balances.size(); ++i) {
				delta.addAccount(addresses[i], Height(1));
				auto& accountState = delta.find(addresses[i]).get();
				accountState.SupplementalPublicKeys.vrf().set(vrfPublicKeys[i]);
				accountState.SupplementalPublicKeys.voting().add(votingPublicKeys[i]);
				accountState.Balances.credit(Harvesting_Mosaic_Id, balances[i]);
			}

			return addresses;
		}
	}

	// endregion

	// region mismatch save calls

	namespace {
		template<typename TSaveFunc>
		void PrepareCannotSaveTest(const CacheConfiguration& cacheConfig, TSaveFunc saveFunc) {
			// Arrange:
			AccountStateCacheTypes::Options options;
			options.HarvestingMosaicId = Harvesting_Mosaic_Id;

			AccountStateCacheSubCachePlugin plugin(cacheConfig, options);
			auto pStorage = plugin.createStorage();

			auto catapultCache = test::CoreSystemCacheFactory::Create(CreateConfigurationFromOptions(options));

			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);

			// Act + Assert:
			saveFunc(*pStorage, catapultCache, stream);
		}
	}

	TEST(TEST_CLASS, Full_CannotSaveSummary) {
		// Arrange:
		PrepareCannotSaveTest(CacheConfiguration(), [](auto& storage, auto& catapultCache, auto& stream) {
			auto cacheDelta = catapultCache.createDelta();

			// Act + Assert:
			EXPECT_THROW(storage.saveSummary(cacheDelta, stream), catapult_invalid_argument);
		});
	}

	TEST(TEST_CLASS, Summary_CannotSaveAll) {
		// Arrange:
		test::TempDirectoryGuard dbDirGuard;
		auto cacheConfig = CacheConfiguration(dbDirGuard.name(), utils::FileSize(), cache::PatriciaTreeStorageMode::Disabled);
		PrepareCannotSaveTest(cacheConfig, [](auto& storage, auto& catapultCache, auto& stream) {
			auto cacheView = catapultCache.createView();

			// Act + Assert:
			EXPECT_THROW(storage.saveAll(cacheView, stream), catapult_invalid_argument);
		});
	}

	// endregion

	// region roundtrip - traits

	namespace {
		struct FullTraits {
			static constexpr auto Should_Load_Accounts = true;

			class CacheConfigurationFactory {
			public:
				CacheConfiguration create() {
					return CacheConfiguration();
				}
			};

			class Saver {
			public:
				explicit Saver(const CacheStorage& storage) : m_storage(storage)
				{}

			public:
				bool save(const CatapultCacheDelta&, io::OutputStream&) {
					return false;
				}

				void save(const CatapultCacheView& cacheView, io::OutputStream& output) {
					m_storage.saveAll(cacheView, output);
				}

			private:
				const CacheStorage& m_storage;
			};
		};

		struct SummaryTraits {
			static constexpr auto Should_Load_Accounts = false;

			class CacheConfigurationFactory {
			public:
				CacheConfiguration create() {
					return CacheConfiguration(m_dbDirGuard.name(), utils::FileSize(), cache::PatriciaTreeStorageMode::Disabled);
				}

			private:
				test::TempDirectoryGuard m_dbDirGuard;
			};

			class Saver {
			public:
				explicit Saver(const CacheStorage& storage) : m_storage(storage)
				{}

			public:
				bool save(const CatapultCacheDelta& cacheDelta, io::OutputStream& output) {
					m_storage.saveSummary(cacheDelta, output);
					return true;
				}

				void save(const CatapultCacheView&, io::OutputStream&)
				{}

			private:
				const CacheStorage& m_storage;
			};
		};
	}

#define ROUNDTRIP_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, Full_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<FullTraits>(); } \
	TEST(TEST_CLASS, Summary_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<SummaryTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region roundtrip tests

	namespace {
		void SetFinalizationPoints(model::PinnedVotingKey& pinnedPublicKey, uint64_t startPoint, uint64_t endPoint) {
			pinnedPublicKey.StartPoint = FinalizationPoint(startPoint);
			pinnedPublicKey.EndPoint = FinalizationPoint(endPoint);
		}

		template<typename TTraits>
		void RunRoundtripTest(
				const AccountStateCacheTypes::Options& options,
				const std::vector<Amount>& balances,
				const std::function<model::AddressSet (const std::vector<Address>&)>& getExpectedAddresses,
				const std::function<AddressAccountHistoryMap (const std::vector<Address>&)>& getExpectedAccountHistories) {
			// Arrange:
			typename TTraits::CacheConfigurationFactory cacheConfigFactory;
			auto cacheConfig = cacheConfigFactory.create();

			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);

			// Act:
			std::vector<Address> addresses;
			auto vrfPublicKeys = test::GenerateRandomDataVector<Key>(balances.size() + 1);
			auto votingPublicKeys = test::GenerateRandomDataVector<model::PinnedVotingKey>(balances.size() + 1);
			SetFinalizationPoints(votingPublicKeys[balances.size() - 1], 200, 400);
			SetFinalizationPoints(votingPublicKeys.back(), 600, 900);
			{
				AccountStateCacheSubCachePlugin plugin(cacheConfig, options);
				auto pStorage = plugin.createStorage();
				typename TTraits::Saver saver(*pStorage);

				auto catapultCache = test::CoreSystemCacheFactory::Create(CreateConfigurationFromOptions(options));
				{
					auto cacheDelta = catapultCache.createDelta();
					auto& delta = cacheDelta.sub<AccountStateCache>();
					addresses = AddAccountsWithBalances(delta, balances, vrfPublicKeys, votingPublicKeys);
					delta.updateHighValueAccounts(Height(3));

					// - change the first account's balance
					// - this shows (a) balance and key histories are independent (b) deep balance history is stored
					delta.find(addresses.front()).get().Balances.credit(Harvesting_Mosaic_Id, Amount(100'000));
					delta.updateHighValueAccounts(Height(4));

					// - change the last account's supplemental public keys
					// - this shows (a) balance and key histories are independent (b) deep key history is stored
					{
						auto& accountPublicKeys = delta.find(addresses.back()).get().SupplementalPublicKeys;
						accountPublicKeys.vrf().unset();
						accountPublicKeys.vrf().set(vrfPublicKeys.back());

						accountPublicKeys.voting().add(votingPublicKeys.back());
					}

					delta.updateHighValueAccounts(Height(5));

					// Sanity:
					EXPECT_EQ(getExpectedAddresses(addresses).size(), delta.highValueAccounts().addresses().size());
					EXPECT_EQ(getExpectedAccountHistories(addresses).size(), delta.highValueAccounts().accountHistories().size());

					// - save summary
					if (!saver.save(cacheDelta, stream))
						catapultCache.commit(Height(3));
				}

				// - save all
				auto cacheView = catapultCache.createView();
				saver.save(cacheView, stream);
			}

			// - reload to roundtrip
			AccountStateCacheSubCachePlugin plugin(cacheConfig, options);
			auto pStorage = plugin.createStorage();
			stream.seek(0);
			pStorage->loadAll(stream, 1);

			// Assert: all accounts were loaded as appropriate
			auto view = plugin.cache().createView();
			EXPECT_EQ(TTraits::Should_Load_Accounts ? balances.size() : 0u, view->size());

			// - all high value account information was loaded
			EXPECT_EQ(getExpectedAddresses(addresses), view->highValueAccounts().addresses());

			// - augment expected account (balance) histories with expected key histories
			auto expectedAccountHistories = getExpectedAccountHistories(addresses);
			for (auto& accountHistoryPair : expectedAccountHistories) {
				// - augment with original expected public keys
				for (auto i = 0u; i < addresses.size(); ++i) {
					if (addresses[i] == accountHistoryPair.first) {
						accountHistoryPair.second.add(Height(3), vrfPublicKeys[i]);
						accountHistoryPair.second.add(Height(3), { votingPublicKeys[i] });
					}
				}

				// - augment with modified expected public keys
				if (addresses.back() == accountHistoryPair.first) {
					accountHistoryPair.second.add(Height(5), vrfPublicKeys.back());
					accountHistoryPair.second.add(Height(5), { votingPublicKeys[balances.size() - 1], votingPublicKeys.back() });
				}
			}

			test::AssertEqual(expectedAccountHistories, view->highValueAccounts().accountHistories());
		}
	}

	ROUNDTRIP_TEST(CanRoundtripHighValueAddressesOnly) {
		// Arrange:
		AccountStateCacheTypes::Options options;
		options.MinHarvesterBalance = Amount(700'000);
		options.MinVoterBalance = Amount(2'000'000);
		options.HarvestingMosaicId = Harvesting_Mosaic_Id;

		// Act + Assert:
		RunRoundtripTest<TTraits>(
				options,
				{ Amount(1'000'000), Amount(500'000), Amount(750'000), Amount(1'250'000) },
				[](const auto& addresses) {
					return model::AddressSet({ addresses[0], addresses[2], addresses[3] });
				},
				[](const auto&) {
					return AddressAccountHistoryMap();
				});
	}

	ROUNDTRIP_TEST(CanRoundtripAccountHistoriesOnly) {
		// Arrange:
		AccountStateCacheTypes::Options options;
		options.MinHarvesterBalance = Amount(2'000'000);
		options.MinVoterBalance = Amount(1'000'000);
		options.HarvestingMosaicId = Harvesting_Mosaic_Id;

		// Act + Assert:
		RunRoundtripTest<TTraits>(
				options,
				{ Amount(1'000'000), Amount(500'000), Amount(750'000), Amount(1'250'000) },
				[](const auto&) {
					return model::AddressSet();
				},
				[](const auto& addresses) {
					return test::GenerateAccountHistories({
						{ addresses[0], { { Height(3), Amount(1'000'000) }, { Height(4), Amount(1'100'000) } } },
						{ addresses[3], { { Height(3), Amount(1'250'000) } } }
					});
				});
	}

	ROUNDTRIP_TEST(CanRoundtripHighValueAddressesAndAccountHistories) {
		// Arrange:
		AccountStateCacheTypes::Options options;
		options.MinHarvesterBalance = Amount(700'000);
		options.MinVoterBalance = Amount(900'000);
		options.HarvestingMosaicId = Harvesting_Mosaic_Id;

		// Act + Assert:
		RunRoundtripTest<TTraits>(
				options,
				{ Amount(1'000'000), Amount(500'000), Amount(750'000), Amount(1'250'000) },
				[](const auto& addresses) {
					return model::AddressSet({ addresses[0], addresses[2], addresses[3] });
				},
				[](const auto& addresses) {
					return test::GenerateAccountHistories({
						{ addresses[0], { { Height(3), Amount(1'000'000) }, { Height(4), Amount(1'100'000) } } },
						{ addresses[3], { { Height(3), Amount(1'250'000) } } }
					});
				});
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
