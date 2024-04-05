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
#include "src/model/SecretLockReceiptType.h"
#include "catapult/model/Address.h"
#include "plugins/txes/lock_shared/tests/observers/ExpiredLockInfoObserverTests.h"
#include "tests/test/SecretLockInfoCacheTestUtils.h"

namespace catapult { namespace observers {

#define TEST_CLASS ExpiredSecretLockInfoObserverTests

	namespace {
		using HeightUnorderedSet = std::unordered_set<Height, utils::BaseValueHasher<Height>>;
	}

	DEFINE_COMMON_OBSERVER_TESTS(ExpiredSecretLockInfo, HeightUnorderedSet(), HeightUnorderedSet())

	namespace {
		struct ExpiredSecretLockInfoTraits : public test::BasicSecretLockInfoTestTraits {
		public:
			using ObserverTestContext = test::ObserverTestContextT<test::SecretLockInfoCacheFactory>;

			static constexpr auto Receipt_Type = model::Receipt_Type_LockSecret_Expired;

		public:
			static auto CreateObserver() {
				return CreateExpiredSecretLockInfoObserver(HeightUnorderedSet(), HeightUnorderedSet());
			}

			static auto& SubCache(cache::CatapultCacheDelta& cache) {
				return cache.sub<cache::SecretLockInfoCache>();
			}
		};

		using ObserverTests = ExpiredLockInfoObserverTests<ExpiredSecretLockInfoTraits>;
		using SeedTuple = ObserverTests::SeedTuple;

		constexpr auto Harvester_Type = ObserverTests::HarvesterType::Main;
	}

	// region no operation

	namespace {
		// these tests don't stricly require public key instead of address because harvester type is always Main
		// and public key is only required when harvester type is Remote

		Address ToAddress(const Key& publicKey) {
			return model::PublicKeyToAddress(publicKey, model::NetworkIdentifier::Zero);
		}
	}

	TEST(TEST_CLASS, ObserverDoesNothingWhenNoLockInfoExpires_Commit) {
		// Arrange:
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		std::vector<SeedTuple> expiringSeeds;

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Commit, Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ blockHarvester, MosaicId(500), Amount(200), Amount() }
		});
	}

	TEST(TEST_CLASS, ObserverDoesNothingWhenNoLockInfoExpires_Rollback) {
		// Arrange:
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		std::vector<SeedTuple> expiringSeeds;

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Rollback, Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ blockHarvester, MosaicId(500), Amount(200), Amount() }
		});
	}

	// endregion

	// region expiration (single)

	TEST(TEST_CLASS, ObserverCreditsAccountsOnCommit_Single) {
		// Arrange:
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		auto address = test::GenerateRandomByteArray<Address>();
		std::vector<SeedTuple> expiringSeeds{
			{ address, MosaicId(111), Amount(333), Amount(33) }
		};

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Commit, Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ address, MosaicId(111), Amount(333 + 33), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200), Amount() }
		});
	}

	TEST(TEST_CLASS, ObserverCreditsAccountsOnRollback_Single) {
		// Arrange:
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		auto address = test::GenerateRandomByteArray<Address>();
		std::vector<SeedTuple> expiringSeeds{
			{ address, MosaicId(111), Amount(333), Amount(33) }
		};

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Rollback, Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ address, MosaicId(111), Amount(333 - 33), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200), Amount() }
		});
	}

	// endregion

	// region expiration (multiple)

	namespace {
		template<typename TSeedTuple = SeedTuple>
		std::vector<TSeedTuple> CreateExpiringSeedTuplesForMultipleTests(const std::vector<Address>& addresses) {
			return {
				{ addresses[0], MosaicId(111), Amount(333), Amount(33) },
				{ addresses[1], MosaicId(222), Amount(222), Amount(88) },
				{ addresses[2], MosaicId(111), Amount(444), Amount(44) },
				{ addresses[1], MosaicId(222), Amount(), Amount(22) }
			};
		}
	}

	TEST(TEST_CLASS, ObserverCreditsAccountsOnCommit_Multiple) {
		// Arrange:
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		auto addresses = test::GenerateRandomDataVector<Address>(3);
		auto expiringSeeds = CreateExpiringSeedTuplesForMultipleTests(addresses);

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Commit, Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ addresses[0], MosaicId(111), Amount(333 + 33), Amount() },
			{ addresses[1], MosaicId(222), Amount(222 + 88 + 22), Amount() },
			{ addresses[2], MosaicId(111), Amount(444 + 44), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200), Amount() }
		});
	}

	TEST(TEST_CLASS, ObserverCreditsAccountsOnRollback_Multiple) {
		// Arrange:
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		auto addresses = test::GenerateRandomDataVector<Address>(3);
		auto expiringSeeds = CreateExpiringSeedTuplesForMultipleTests(addresses);

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Rollback, Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ addresses[0], MosaicId(111), Amount(333 - 33), Amount() },
			{ addresses[1], MosaicId(222), Amount(222 - 88 - 22), Amount() },
			{ addresses[2], MosaicId(111), Amount(444 - 44), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200), Amount() }
		});
	}

	// endregion

	// region expiration (skipped, forced)

	TEST(TEST_CLASS, ObserverDoesNothingWhenExpirationIsNotOfMostRecentLock) {
		// Arrange:
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		auto addresses = test::GenerateRandomDataVector<Address>(3);
		auto expiringSeeds = CreateExpiringSeedTuplesForMultipleTests(addresses);

		ObserverTests::BalanceTestOptions options;
		options.SetFutureExpiration = true; // expiring seeds will have locks expiring at heights 55 and 65

		// Act + Assert: none of the expiring lock owners are credited (or touched)
		ObserverTests::RunBalanceTest(NotifyMode::Commit, Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ addresses[0], MosaicId(111), Amount(333), Amount() },
			{ addresses[1], MosaicId(222), Amount(222), Amount() },
			{ addresses[2], MosaicId(111), Amount(444), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200), Amount() }
		}, options);
	}

	TEST(TEST_CLASS, ObserverDoesNothingWhenExpirationIsSkippedAtHeight) {
		// Arrange:
		struct CustomExpiredSecretLockInfoTraits : public ExpiredSecretLockInfoTraits {
		public:
			static auto CreateObserver() {
				return CreateExpiredSecretLockInfoObserver(
						HeightUnorderedSet{ Height(55) }, // context height used in ExpiredLockInfoObserverTests::RunBalanceTest
						HeightUnorderedSet());
			}
		};

		using CustomObserverTests = ExpiredLockInfoObserverTests<CustomExpiredSecretLockInfoTraits>;
		constexpr auto Custom_Harvester_Type = CustomObserverTests::HarvesterType::Main;

		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		auto addresses = test::GenerateRandomDataVector<Address>(3);
		auto expiringSeeds = CreateExpiringSeedTuplesForMultipleTests<CustomObserverTests::SeedTuple>(addresses);

		CustomObserverTests::BalanceTestOptions options;
		options.ExpectTouches = false;

		// Act + Assert: none of the expiring lock owners are credited (or touched)
		CustomObserverTests::RunBalanceTest(NotifyMode::Commit, Custom_Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ addresses[0], MosaicId(111), Amount(333), Amount() },
			{ addresses[1], MosaicId(222), Amount(222), Amount() },
			{ addresses[2], MosaicId(111), Amount(444), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200), Amount() }
		}, options);
	}

	TEST(TEST_CLASS, ObserverCreditsAccountsOnCommitWhenExpirationIsNotOfMostRecentLockButForced) {
		// Arrange:
		struct CustomExpiredSecretLockInfoTraits : public ExpiredSecretLockInfoTraits {
		public:
			static auto CreateObserver() {
				return CreateExpiredSecretLockInfoObserver(
						HeightUnorderedSet(),
						HeightUnorderedSet{ Height(55) }); // context height used in ExpiredLockInfoObserverTests::RunBalanceTest
			}
		};

		using CustomObserverTests = ExpiredLockInfoObserverTests<CustomExpiredSecretLockInfoTraits>;
		constexpr auto Custom_Harvester_Type = CustomObserverTests::HarvesterType::Main;

		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();
		auto blockHarvester = ToAddress(blockHarvesterPublicKey);

		auto addresses = test::GenerateRandomDataVector<Address>(3);
		auto expiringSeeds = CreateExpiringSeedTuplesForMultipleTests<CustomObserverTests::SeedTuple>(addresses);

		CustomObserverTests::BalanceTestOptions options;
		options.SetFutureExpiration = true; // expiring seeds will have locks expiring at heights 55 and 65

		// Act + Assert: the expired lock owners are credited despite an old lock expiration
		CustomObserverTests::RunBalanceTest(NotifyMode::Commit, Custom_Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ addresses[0], MosaicId(111), Amount(333 + 33), Amount() },
			{ addresses[1], MosaicId(222), Amount(222 + 88 + 22), Amount() },
			{ addresses[2], MosaicId(111), Amount(444 + 44), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200), Amount() }
		}, options);
	}

	// endregion

	// region receipts (multiple)

	TEST(TEST_CLASS, ObserverCreatesReceiptsOnCommit) {
		// Arrange:
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();

		std::vector<SeedTuple> expiringSeeds{
			{ Address{ { 9 } }, MosaicId(111), Amount(333), Amount(33) },
			{ Address{ { 4 } }, MosaicId(222), Amount(222), Amount(88) },
			{ Address{ { 4 } }, MosaicId(111), Amount(444), Amount(33) },
			{ Address{ { 9 } }, MosaicId(222), Amount(), Amount(22) }
		};

		// Act + Assert: notice that receipts are deterministically ordered
		ObserverTests::RunReceiptTest(NotifyMode::Commit, Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {
			{ Address{ { 4 } }, MosaicId(111), Amount(), Amount(33) },
			{ Address{ { 9 } }, MosaicId(111), Amount(), Amount(33) },
			{ Address{ { 9 } }, MosaicId(222), Amount(), Amount(22) },
			{ Address{ { 4 } }, MosaicId(222), Amount(), Amount(88) }
		});
	}

	TEST(TEST_CLASS, ObserverDoesNotCreateReceiptsOnRollback) {
		// Arrange:
		auto blockHarvesterPublicKey = test::GenerateRandomByteArray<Key>();

		std::vector<SeedTuple> expiringSeeds{
			{ Address{ { 9 } }, MosaicId(111), Amount(333), Amount(33) },
			{ Address{ { 1 } }, MosaicId(222), Amount(222), Amount(88) },
			{ Address{ { 4 } }, MosaicId(111), Amount(444), Amount(44) },
			{ Address{ { 1 } }, MosaicId(222), Amount(), Amount(22) }
		};

		// Act + Assert:
		ObserverTests::RunReceiptTest(NotifyMode::Rollback, Harvester_Type, blockHarvesterPublicKey, expiringSeeds, {});
	}

	// endregion
}}
