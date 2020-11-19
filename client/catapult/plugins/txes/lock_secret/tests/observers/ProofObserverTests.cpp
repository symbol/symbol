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
#include "plugins/txes/lock_shared/tests/observers/LockStatusAndBalanceObserverTests.h"
#include "tests/test/SecretLockNotificationsTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"

namespace catapult { namespace observers {

#define TEST_CLASS ProofObserverTests

	using ObserverTestContext = test::ObserverTestContextT<test::SecretLockInfoCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(Proof,)

	namespace {
		struct SecretTraits {
		public:
			using BasicTraits = test::BasicSecretLockInfoTestTraits;
			using NotificationBuilder = test::ProofNotificationBuilder;
			using ObserverTestContext = observers::ObserverTestContext;

			static constexpr auto Receipt_Type = model::Receipt_Type_LockSecret_Completed;

			static auto CreateObserver() {
				return CreateProofObserver();
			}

			static auto DestinationAccount(const BasicTraits::ValueType& lockInfo) {
				return lockInfo.RecipientAddress;
			}
		};
	}

	DEFINE_LOCK_STATUS_OBSERVER_TESTS(SecretTraits)
}}
