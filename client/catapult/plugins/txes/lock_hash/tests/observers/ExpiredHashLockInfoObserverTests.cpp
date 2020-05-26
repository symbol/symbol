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
#include "src/model/HashLockReceiptType.h"
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

	// region no operation

	TEST(TEST_CLASS, ObserverDoesNothingWhenNoLockInfoExpires_Commit) {
		// Arrange:
		auto blockHarvester = test::GenerateRandomByteArray<Address>();
		std::vector<SeedTuple> expiringSeeds;

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Commit, blockHarvester, expiringSeeds, {
			{ blockHarvester, MosaicId(500), Amount(200), Amount() }
		});
	}

	TEST(TEST_CLASS, ObserverDoesNothingWhenNoLockInfoExpires_Rollback) {
		// Arrange:
		auto blockHarvester = test::GenerateRandomByteArray<Address>();
		std::vector<SeedTuple> expiringSeeds;

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Rollback, blockHarvester, expiringSeeds, {
			{ blockHarvester, MosaicId(500), Amount(200), Amount() }
		});
	}

	// endregion

	// region expiration (single)

	TEST(TEST_CLASS, ObserverCreditsAccountsOnCommit_Single) {
		// Arrange:
		auto blockHarvester = test::GenerateRandomByteArray<Address>();
		auto address = test::GenerateRandomByteArray<Address>();
		std::vector<SeedTuple> expiringSeeds{
			{ address, MosaicId(500), Amount(333), Amount(33) }
		};

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Commit, blockHarvester, expiringSeeds, {
			{ address, MosaicId(500), Amount(333), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200 + 33), Amount() }
		});
	}

	TEST(TEST_CLASS, ObserverCreditsAccountsOnRollback_Single) {
		// Arrange:
		auto blockHarvester = test::GenerateRandomByteArray<Address>();
		auto address = test::GenerateRandomByteArray<Address>();
		std::vector<SeedTuple> expiringSeeds{
			{ address, MosaicId(500), Amount(333), Amount(33) }
		};

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Rollback, blockHarvester, expiringSeeds, {
			{ address, MosaicId(500), Amount(333), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200 - 33), Amount() }
		});
	}

	// endregion

	// region expiration (multiple)

	TEST(TEST_CLASS, ObserverCreditsAccountsOnCommit_Multiple) {
		// Arrange: using single mosaic id to emulate typical operation
		auto blockHarvester = test::GenerateRandomByteArray<Address>();
		auto addresses = test::GenerateRandomDataVector<Address>(3);
		std::vector<SeedTuple> expiringSeeds{
			{ addresses[0], MosaicId(500), Amount(333), Amount(33) },
			{ addresses[1], MosaicId(500), Amount(222), Amount(88) },
			{ addresses[2], MosaicId(500), Amount(444), Amount(44) },
			{ addresses[1], MosaicId(500), Amount(), Amount(22) }
		};

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Commit, blockHarvester, expiringSeeds, {
			{ addresses[0], MosaicId(500), Amount(333), Amount() },
			{ addresses[1], MosaicId(500), Amount(222), Amount() },
			{ addresses[2], MosaicId(500), Amount(444), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200 + 33 + 88 + 44 + 22), Amount() }
		});
	}

	TEST(TEST_CLASS, ObserverCreditsAccountsOnRollback_Multiple) {
		// Arrange: using single mosaic id to emulate typical operation
		auto blockHarvester = test::GenerateRandomByteArray<Address>();
		auto addresses = test::GenerateRandomDataVector<Address>(3);
		std::vector<SeedTuple> expiringSeeds{
			{ addresses[0], MosaicId(500), Amount(333), Amount(33) },
			{ addresses[1], MosaicId(500), Amount(222), Amount(88) },
			{ addresses[2], MosaicId(500), Amount(444), Amount(44) },
			{ addresses[1], MosaicId(500), Amount(), Amount(22) }
		};

		// Act + Assert:
		ObserverTests::RunBalanceTest(NotifyMode::Rollback, blockHarvester, expiringSeeds, {
			{ addresses[0], MosaicId(500), Amount(333), Amount() },
			{ addresses[1], MosaicId(500), Amount(222), Amount() },
			{ addresses[2], MosaicId(500), Amount(444), Amount() },
			{ blockHarvester, MosaicId(500), Amount(200 - 33 - 88 - 44 - 22), Amount() }
		});
	}

	// endregion

	// region receipts (multiple)

	TEST(TEST_CLASS, ObserverCreatesReceiptsOnCommit) {
		// Arrange: using single mosaic id to emulate typical operation
		auto blockHarvester = test::GenerateRandomByteArray<Address>();
		std::vector<SeedTuple> expiringSeeds{
			{ Address{ { 9 } }, MosaicId(500), Amount(333), Amount(33) },
			{ Address{ { 1 } }, MosaicId(500), Amount(222), Amount(88) },
			{ Address{ { 4 } }, MosaicId(500), Amount(444), Amount(44) },
			{ Address{ { 1 } }, MosaicId(500), Amount(), Amount(22) }
		};

		// Act + Assert: notice that receipts are deterministically ordered
		ObserverTests::RunReceiptTest(NotifyMode::Commit, blockHarvester, expiringSeeds, {
			{ blockHarvester, MosaicId(500), Amount(), Amount(22) },
			{ blockHarvester, MosaicId(500), Amount(), Amount(33) },
			{ blockHarvester, MosaicId(500), Amount(), Amount(44) },
			{ blockHarvester, MosaicId(500), Amount(), Amount(88) }
		});
	}

	TEST(TEST_CLASS, ObserverDoesNotCreateReceiptsOnRollback) {
		// Arrange: using single mosaic id to emulate typical operation
		auto blockHarvester = test::GenerateRandomByteArray<Address>();
		std::vector<SeedTuple> expiringSeeds{
			{ Address{ { 9 } }, MosaicId(500), Amount(333), Amount(33) },
			{ Address{ { 1 } }, MosaicId(500), Amount(222), Amount(88) },
			{ Address{ { 4 } }, MosaicId(500), Amount(444), Amount(44) },
			{ Address{ { 1 } }, MosaicId(500), Amount(), Amount(22) }
		};

		// Act + Assert:
		ObserverTests::RunReceiptTest(NotifyMode::Rollback, blockHarvester, expiringSeeds, {});
	}

	// endregion
}}
