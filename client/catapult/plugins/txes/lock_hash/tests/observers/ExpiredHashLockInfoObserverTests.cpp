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

#include "src/observers/Observers.h"
#include "src/model/HashLockReceiptType.h"
#include "catapult/model/Address.h"
#include "plugins/txes/lock_shared/tests/observers/ExpiredLockInfoObserverTests.h"
#include "tests/test/HashLockInfoCacheTestUtils.h"

namespace catapult { namespace observers {

#define TEST_CLASS ExpiredHashLockInfoObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(ExpiredHashLockInfo,)

	namespace {
		struct ExpiredHashLockInfoTraits : public test::BasicHashLockInfoTestTraits {
		public:
			using ObserverTestContext = test::ObserverTestContextT<test::HashLockInfoCacheFactory>;

			static constexpr auto Receipt_Type = model::Receipt_Type_LockHash_Expired;

		public:
			static auto CreateObserver() {
				return CreateExpiredHashLockInfoObserver();
			}

			static auto& SubCache(cache::CatapultCacheDelta& cache) {
				return cache.sub<cache::HashLockInfoCache>();
			}
		};

		using ObserverTests = ExpiredLockInfoObserverTests<ExpiredHashLockInfoTraits>;
		using SeedTuple = ObserverTests::SeedTuple;
	}

	// region traits

	namespace {
		struct MainHarvesterTraits {
			static constexpr auto Harvester_Type = ObserverTests::HarvesterType::Main;
		};

		struct RemoteHarvesterTraits {
			static constexpr auto Harvester_Type = ObserverTests::HarvesterType::Remote;
		};
	}

#define HARVESTER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_MainHarvester) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MainHarvesterTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_RemoteHarvester) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RemoteHarvesterTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region no operation

	namespace {
		Address ToAddress(const Key& publicKey) {
			return model::PublicKeyToAddress(publicKey, model::NetworkIdentifier::Zero);
		}
	}

	HARVESTER_TEST(ObserverDoesNothingWhenNoLockInfoExpires_Commit) {
		// Arrange:
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		std::vector<SeedTuple> expiringSeeds;

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Commit, TTraits::Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ blockHarvester, MosaicId(500), Amount(200), Amount() }
		});
	}

	HARVESTER_TEST(ObserverDoesNothingWhenNoLockInfoExpires_Rollback) {
		// Arrange:
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		std::vector<SeedTuple> expiringSeeds;

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Rollback, TTraits::Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ blockHarvester, MosaicId(500), Amount(200), Amount() }
		});
	}

	// endregion

	// region expiration (single)

	HARVESTER_TEST(ObserverCreditsAccountsOnCommit_Single) {
		// Arrange:
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		auto address = test::GenerateRandomByteArray<Address>();
		std::vector<SeedTuple> expiringSeeds{
			{ address, MosaicId(500), Amount(333), Amount(33) }
		};

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Commit, TTraits::Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ address, MosaicId(500), Amount(333), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200 + 33), Amount() }
		});
	}

	HARVESTER_TEST(ObserverCreditsAccountsOnRollback_Single) {
		// Arrange:
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		auto address = test::GenerateRandomByteArray<Address>();
		std::vector<SeedTuple> expiringSeeds{
			{ address, MosaicId(500), Amount(333), Amount(33) }
		};

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Rollback, TTraits::Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ address, MosaicId(500), Amount(333), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200 - 33), Amount() }
		});
	}

	// endregion

	// region expiration (multiple)

	namespace {
		std::vector<SeedTuple> CreateExpiringSeedTuplesForMultipleTests(const std::vector<Address>& addresses) {
			return {
				{ addresses[0], MosaicId(500), Amount(333), Amount(33) },
				{ addresses[1], MosaicId(500), Amount(222), Amount(88) },
				{ addresses[2], MosaicId(500), Amount(444), Amount(44) },
				{ addresses[1], MosaicId(500), Amount(), Amount(22) }
			};
		}
	}

	HARVESTER_TEST(ObserverCreditsAccountsOnCommit_Multiple) {
		// Arrange: using single mosaic id to emulate typical operation
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		auto addresses = test::GenerateRandomDataVector<Address>(3);
		auto expiringSeeds = CreateExpiringSeedTuplesForMultipleTests(addresses);

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Commit, TTraits::Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ addresses[0], MosaicId(500), Amount(333), Amount() },
			{ addresses[1], MosaicId(500), Amount(222), Amount() },
			{ addresses[2], MosaicId(500), Amount(444), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200 + 33 + 88 + 44 + 22), Amount() }
		});
	}

	HARVESTER_TEST(ObserverCreditsAccountsOnRollback_Multiple) {
		// Arrange: using single mosaic id to emulate typical operation
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		auto addresses = test::GenerateRandomDataVector<Address>(3);
		auto expiringSeeds = CreateExpiringSeedTuplesForMultipleTests(addresses);

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Rollback, TTraits::Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ addresses[0], MosaicId(500), Amount(333), Amount() },
			{ addresses[1], MosaicId(500), Amount(222), Amount() },
			{ addresses[2], MosaicId(500), Amount(444), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200 - 33 - 88 - 44 - 22), Amount() }
		});
	}

	// endregion

	// region expiration (skipped)

	HARVESTER_TEST(ObserverDoesNothingWhenExpirationIsNotOfMostRecentLock) {
		// Arrange:
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		auto addresses = test::GenerateRandomDataVector<Address>(3);
		auto expiringSeeds = CreateExpiringSeedTuplesForMultipleTests(addresses);

		ObserverTests::BalanceTestOptions options;
		options.SetFutureExpiration = true; // expiring seeds will have locks expiring at heights 55 and 65

		// Act + Assert: the harvester is not credited
		ObserverTests::RunBalanceTest(NotifyMode::Commit, TTraits::Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ addresses[0], MosaicId(500), Amount(333), Amount() },
			{ addresses[1], MosaicId(500), Amount(222), Amount() },
			{ addresses[2], MosaicId(500), Amount(444), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200), Amount() }
		}, options);
	}

	// endregion

	// region receipts (multiple)

	HARVESTER_TEST(ObserverCreatesReceiptsOnCommit) {
		// Arrange: using single mosaic id to emulate typical operation
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		std::vector<SeedTuple> expiringSeeds{
			{ Address{ { 9 } }, MosaicId(500), Amount(333), Amount(33) },
			{ Address{ { 1 } }, MosaicId(500), Amount(222), Amount(88) },
			{ Address{ { 4 } }, MosaicId(500), Amount(444), Amount(44) },
			{ Address{ { 1 } }, MosaicId(500), Amount(), Amount(22) }
		};

		// Act + Assert: notice that receipts are deterministically ordered
		ObserverTests::RunReceiptTest(NotifyMode::Commit, TTraits::Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ blockHarvester, MosaicId(500), Amount(), Amount(22) },
			{ blockHarvester, MosaicId(500), Amount(), Amount(33) },
			{ blockHarvester, MosaicId(500), Amount(), Amount(44) },
			{ blockHarvester, MosaicId(500), Amount(), Amount(88) }
		});
	}

	HARVESTER_TEST(ObserverDoesNotCreateReceiptsOnRollback) {
		// Arrange: using single mosaic id to emulate typical operation
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();

		std::vector<SeedTuple> expiringSeeds{
			{ Address{ { 9 } }, MosaicId(500), Amount(333), Amount(33) },
			{ Address{ { 1 } }, MosaicId(500), Amount(222), Amount(88) },
			{ Address{ { 4 } }, MosaicId(500), Amount(444), Amount(44) },
			{ Address{ { 1 } }, MosaicId(500), Amount(), Amount(22) }
		};

		// Act + Assert:
		ObserverTests::RunReceiptTest(NotifyMode::Rollback, TTraits::Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {});
	}

	// endregion
}}
