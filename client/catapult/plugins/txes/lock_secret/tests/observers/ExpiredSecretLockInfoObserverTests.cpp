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

#include "src/observers/Observers.h"
#include "src/model/SecretLockReceiptType.h"
#include "plugins/txes/lock_shared/tests/observers/ExpiredLockInfoObserverTests.h"
#include "tests/test/SecretLockInfoCacheTestUtils.h"

namespace catapult { namespace observers {

#define TEST_CLASS ExpiredSecretLockInfoObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(ExpiredSecretLockInfo,)

	namespace {
		struct ExpiredSecretLockInfoTraits : public test::BasicSecretLockInfoTestTraits {
		public:
			using ObserverTestContext = test::ObserverTestContextT<test::SecretLockInfoCacheFactory>;

			static constexpr auto Receipt_Type = model::Receipt_Type_LockSecret_Expired;

		public:
			static auto CreateObserver() {
				return CreateExpiredSecretLockInfoObserver();
			}

			static auto& SubCache(cache::CatapultCacheDelta& cache) {
				return cache.sub<cache::SecretLockInfoCache>();
			}
		};

		using ObserverTests = ExpiredLockInfoObserverTests<ExpiredSecretLockInfoTraits>;
		using SeedTuple = ObserverTests::SeedTuple;
	}

	// region no operation

	TEST(TEST_CLASS, ObserverDoesNothingWhenNoLockInfoExpires_Commit) {
		// Arrange:
		auto blockSigner = test::GenerateRandomByteArray<Key>();
		std::vector<SeedTuple> expiringSeeds;

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Commit, blockSigner, expiringSeeds, {
			{ blockSigner, MosaicId(500), Amount(200), Amount() }
		});
	}

	TEST(TEST_CLASS, ObserverDoesNothingWhenNoLockInfoExpires_Rollback) {
		// Arrange:
		auto blockSigner = test::GenerateRandomByteArray<Key>();
		std::vector<SeedTuple> expiringSeeds;

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Rollback, blockSigner, expiringSeeds, {
			{ blockSigner, MosaicId(500), Amount(200), Amount() }
		});
	}

	// endregion

	// region expiration (single)

	TEST(TEST_CLASS, ObserverCreditsAccountsOnCommit_Single) {
		// Arrange:
		auto blockSigner = test::GenerateRandomByteArray<Key>();
		auto key = test::GenerateRandomByteArray<Key>();
		std::vector<SeedTuple> expiringSeeds{
			{ key, MosaicId(111), Amount(333), Amount(33) }
		};

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Commit, blockSigner, expiringSeeds, {
			{ key, MosaicId(111), Amount(333 + 33), Amount() },
			{ blockSigner, MosaicId(500), Amount(200), Amount() }
		});
	}

	TEST(TEST_CLASS, ObserverCreditsAccountsOnRollback_Single) {
		// Arrange:
		auto blockSigner = test::GenerateRandomByteArray<Key>();
		auto key = test::GenerateRandomByteArray<Key>();
		std::vector<SeedTuple> expiringSeeds{
			{ key, MosaicId(111), Amount(333), Amount(33) }
		};

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Rollback, blockSigner, expiringSeeds, {
			{ key, MosaicId(111), Amount(333 - 33), Amount() },
			{ blockSigner, MosaicId(500), Amount(200), Amount() }
		});
	}

	// endregion

	// region expiration (multiple)

	TEST(TEST_CLASS, ObserverCreditsAccountsOnCommit_Multiple) {
		// Arrange:
		auto blockSigner = test::GenerateRandomByteArray<Key>();
		auto keys = test::GenerateRandomDataVector<Key>(3);
		std::vector<SeedTuple> expiringSeeds{
			{ keys[0], MosaicId(111), Amount(333), Amount(33) },
			{ keys[1], MosaicId(222), Amount(222), Amount(88) },
			{ keys[2], MosaicId(111), Amount(444), Amount(44) },
			{ keys[1], MosaicId(222), Amount(), Amount(22) }
		};

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Commit, blockSigner, expiringSeeds, {
			{ keys[0], MosaicId(111), Amount(333 + 33), Amount() },
			{ keys[1], MosaicId(222), Amount(222 + 88 + 22), Amount() },
			{ keys[2], MosaicId(111), Amount(444 + 44), Amount() },
			{ blockSigner, MosaicId(500), Amount(200), Amount() }
		});
	}

	TEST(TEST_CLASS, ObserverCreditsAccountsOnRollback_Multiple) {
		// Arrange:
		auto blockSigner = test::GenerateRandomByteArray<Key>();
		auto keys = test::GenerateRandomDataVector<Key>(3);
		std::vector<SeedTuple> expiringSeeds{
			{ keys[0], MosaicId(111), Amount(333), Amount(33) },
			{ keys[1], MosaicId(222), Amount(222), Amount(88) },
			{ keys[2], MosaicId(111), Amount(444), Amount(44) },
			{ keys[1], MosaicId(222), Amount(), Amount(22) }
		};

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Rollback, blockSigner, expiringSeeds, {
			{ keys[0], MosaicId(111), Amount(333 - 33), Amount() },
			{ keys[1], MosaicId(222), Amount(222 - 88 - 22), Amount() },
			{ keys[2], MosaicId(111), Amount(444 - 44), Amount() },
			{ blockSigner, MosaicId(500), Amount(200), Amount() }
		});
	}

	// endregion

	// region receipts (multiple)

	TEST(TEST_CLASS, ObserverCreatesReceiptsOnCommit) {
		// Arrange:
		auto blockSigner = test::GenerateRandomByteArray<Key>();
		std::vector<SeedTuple> expiringSeeds{
			{ Key{ { 9 } }, MosaicId(111), Amount(333), Amount(33) },
			{ Key{ { 1 } }, MosaicId(222), Amount(222), Amount(88) },
			{ Key{ { 4 } }, MosaicId(111), Amount(444), Amount(44) },
			{ Key{ { 1 } }, MosaicId(222), Amount(), Amount(22) }
		};

		// Act + Assert: notice that receipts are deterministically ordered
		ObserverTests::RunReceiptTest(NotifyMode::Commit, blockSigner, expiringSeeds, {
			{ Key{ { 1 } }, MosaicId(222), Amount(), Amount(22) },
			{ Key{ { 1 } }, MosaicId(222), Amount(), Amount(88) },
			{ Key{ { 4 } }, MosaicId(111), Amount(), Amount(44) },
			{ Key{ { 9 } }, MosaicId(111), Amount(), Amount(33) }
		});
	}

	TEST(TEST_CLASS, ObserverDoesNotCreateReceiptsOnRollback) {
		// Arrange:
		auto blockSigner = test::GenerateRandomByteArray<Key>();
		std::vector<SeedTuple> expiringSeeds{
			{ Key{ { 9 } }, MosaicId(111), Amount(333), Amount(33) },
			{ Key{ { 1 } }, MosaicId(222), Amount(222), Amount(88) },
			{ Key{ { 4 } }, MosaicId(111), Amount(444), Amount(44) },
			{ Key{ { 1 } }, MosaicId(222), Amount(), Amount(22) }
		};

		// Act + Assert:
		ObserverTests::RunReceiptTest(NotifyMode::Rollback, blockSigner, expiringSeeds, {});
	}

	// endregion
}}
