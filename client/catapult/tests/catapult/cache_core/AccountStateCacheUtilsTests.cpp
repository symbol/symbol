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

#include "catapult/cache_core/AccountStateCacheUtils.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AccountStateCacheUtilsTests

	namespace {
		class TestContext {
		public:
			TestContext()
					: m_cache(CacheConfiguration(), test::CreateDefaultAccountStateCacheOptions())
					, m_pDelta(m_cache.createDelta())
			{}

		public:
			auto& cache() {
				return *m_pDelta;
			}

			auto addAccount(const Key& publicKey) {
				auto& cacheDelta = cache();
				cacheDelta.addAccount(publicKey, Height(123));
				return cacheDelta.find(publicKey);
			}

			void addAccount(const Key& publicKey, state::AccountType accountType, const Key& remoteKey) {
				auto accountStateIter = addAccount(publicKey);
				accountStateIter.get().AccountType = accountType;
				accountStateIter.get().LinkedAccountKey = remoteKey;
			}

		private:
			cache::AccountStateCache m_cache;
			cache::LockedCacheDelta<cache::AccountStateCacheDelta> m_pDelta;
		};
	}

	// region successful forward

	TEST(TEST_CLASS, CanForwardUnlinkedAccount) {
		// Arrange:
		TestContext context;
		auto publicKey = test::GenerateRandomByteArray<Key>();
		context.addAccount(publicKey);

		// Act:
		ProcessForwardedAccountState(context.cache(), publicKey, [&publicKey](const auto& accountState) {
			// Assert:
			EXPECT_EQ(publicKey, accountState.PublicKey);
		});
	}

	TEST(TEST_CLASS, CanForwardMainAccount) {
		// Arrange:
		TestContext context;
		auto publicKey = test::GenerateRandomByteArray<Key>();
		context.addAccount(publicKey, state::AccountType::Main, test::GenerateRandomByteArray<Key>());

		// Act:
		ProcessForwardedAccountState(context.cache(), publicKey, [&publicKey](const auto& accountState) {
			// Assert:
			EXPECT_EQ(publicKey, accountState.PublicKey);
		});
	}

	TEST(TEST_CLASS, CanForwardRemoteAccount) {
		// Arrange:
		TestContext context;
		auto publicKey = test::GenerateRandomByteArray<Key>();
		auto remoteKey = test::GenerateRandomByteArray<Key>();
		context.addAccount(publicKey, state::AccountType::Main, remoteKey);
		context.addAccount(remoteKey, state::AccountType::Remote, publicKey);

		// Act:
		ProcessForwardedAccountState(context.cache(), remoteKey, [&publicKey](const auto& accountState) {
			// Assert: main account is returned
			EXPECT_EQ(publicKey, accountState.PublicKey);
		});
	}

	// endregion

	// region forward failure

	TEST(TEST_CLASS, CannotForwardWhenMainAccountIsNotPresent) {
		// Arrange:
		TestContext context;
		auto publicKey = test::GenerateRandomByteArray<Key>();
		auto remoteKey = test::GenerateRandomByteArray<Key>();
		context.addAccount(remoteKey, state::AccountType::Remote, publicKey);

		// Act + Assert:
		auto& accountStateCache = context.cache();
		EXPECT_THROW(ProcessForwardedAccountState(accountStateCache, remoteKey, [](const auto&) {}), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotForwardWhenMainHasInvalidAccountType) {
		// Arrange:
		TestContext context;
		auto publicKey = test::GenerateRandomByteArray<Key>();
		auto remoteKey = test::GenerateRandomByteArray<Key>();
		context.addAccount(publicKey, state::AccountType::Remote, remoteKey);
		context.addAccount(remoteKey, state::AccountType::Remote, publicKey);

		// Act + Assert:
		auto& accountStateCache = context.cache();
		EXPECT_THROW(ProcessForwardedAccountState(accountStateCache, remoteKey, [](const auto&) {}), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotForwardWhenMainHasInvalidKey) {
		// Arrange: main account does not link-back to remote key
		TestContext context;
		auto publicKey = test::GenerateRandomByteArray<Key>();
		auto remoteKey = test::GenerateRandomByteArray<Key>();
		context.addAccount(publicKey, state::AccountType::Main, test::GenerateRandomByteArray<Key>());
		context.addAccount(remoteKey, state::AccountType::Remote, publicKey);

		// Act + Assert:
		auto& accountStateCache = context.cache();
		EXPECT_THROW(ProcessForwardedAccountState(accountStateCache, remoteKey, [](const auto&) {}), catapult_runtime_error);
	}

	// endregion
}}
