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
#include "plugins/txes/lock_shared/tests/observers/ExpiredLockInfoObserverTests.h"
#include "tests/test/HashLockInfoCacheTestUtils.h"

namespace catapult { namespace observers {

#define TEST_CLASS ExpiredHashLockInfoObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(ExpiredHashLockInfo, )

	namespace {
		struct ExpiredHashLockInfoTraits : public test::BasicHashLockInfoTestTraits {
		public:
			using ObserverTestContext = test::ObserverTestContextT<test::HashLockInfoCacheFactory>;

		public:
			static MosaicId LockInfoMosaicId() {
				return Xem_Id;
			}

			static auto CreateObserver() {
				return CreateExpiredHashLockInfoObserver();
			}

			static auto& SubCache(cache::CatapultCacheDelta& cache) {
				return cache.sub<cache::HashLockInfoCache>();
			}

			static Amount GetExpectedExpiringLockOwnerBalance(observers::NotifyMode, Amount initialBalance, Amount) {
				return initialBalance;
			}

			static Amount GetExpectedBlockSignerBalance(observers::NotifyMode mode, Amount initialBalance, Amount delta, size_t count) {
				// expiring hash lock is paid to block signer
				auto adjustedDelta = Amount(count * delta.unwrap());
				return observers::NotifyMode::Commit == mode ? initialBalance + adjustedDelta : initialBalance - adjustedDelta;
			}
		};
	}

	DEFINE_EXPIRED_LOCK_INFO_OBSERVER_TESTS(ExpiredHashLockInfoTraits)
}}
