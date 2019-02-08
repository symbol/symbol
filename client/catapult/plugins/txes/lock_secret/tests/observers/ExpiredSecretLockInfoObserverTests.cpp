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
#include "tests/test/SecretLockInfoCacheTestUtils.h"

namespace catapult { namespace observers {

#define TEST_CLASS ExpiredSecretLockInfoObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(ExpiredSecretLockInfo,)

	namespace {
		struct ExpiredSecretLockInfoTraits : public test::BasicSecretLockInfoTestTraits {
		public:
			using ObserverTestContext = test::ObserverTestContextT<test::SecretLockInfoCacheFactory>;

		public:
			static auto CreateObserver() {
				return CreateExpiredSecretLockInfoObserver();
			}

			static auto& SubCache(cache::CatapultCacheDelta& cache) {
				return cache.sub<cache::SecretLockInfoCache>();
			}

			static Amount GetExpectedExpiringLockOwnerBalance(observers::NotifyMode mode, Amount initialBalance, Amount delta) {
				// expiring secret lock is paid to lock creator
				return observers::NotifyMode::Commit == mode ? initialBalance + delta : initialBalance - delta;
			}

			static Amount GetExpectedBlockSignerBalance(observers::NotifyMode, Amount initialBalance, Amount, size_t) {
				return initialBalance;
			}
		};
	}

	DEFINE_EXPIRED_LOCK_INFO_OBSERVER_TESTS(ExpiredSecretLockInfoTraits)
}}
