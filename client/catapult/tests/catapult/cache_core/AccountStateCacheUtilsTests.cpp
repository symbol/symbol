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

	// region test context

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
				accountStateIter.get().SupplementalAccountKeys.linkedPublicKey().set(remoteKey);
			}

		private:
			cache::AccountStateCache m_cache;
			cache::LockedCacheDelta<cache::AccountStateCacheDelta> m_pDelta;
		};
	}

	// endregion

	// region traits

	namespace {
		struct DeltaTraits {
			static cache::AccountStateCacheDelta& GetCache(TestContext& context) {
				return context.cache();
			}
		};

		struct ReadOnlyTraits {
			static cache::ReadOnlyAccountStateCache GetCache(TestContext& context) {
				return context.cache().asReadOnly();
			}
		};
	}

#define CACHE_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Delta) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DeltaTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ReadOnly) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ReadOnlyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region successful forward

	CACHE_BASED_TEST(CanForwardUnlinkedAccount) {
		// Arrange:
		TestContext context;
		auto publicKey = test::GenerateRandomByteArray<Key>();
		context.addAccount(publicKey);

		// Act:
		ProcessForwardedAccountState(TTraits::GetCache(context), publicKey, [&publicKey](const auto& accountState) {
			// Assert:
			EXPECT_EQ(publicKey, accountState.PublicKey);
		});
	}

	CACHE_BASED_TEST(CanForwardMainAccount) {
		// Arrange:
		TestContext context;
		auto publicKey = test::GenerateRandomByteArray<Key>();
		context.addAccount(publicKey, state::AccountType::Main, test::GenerateRandomByteArray<Key>());

		// Act:
		ProcessForwardedAccountState(TTraits::GetCache(context), publicKey, [&publicKey](const auto& accountState) {
			// Assert:
			EXPECT_EQ(publicKey, accountState.PublicKey);
		});
	}

	CACHE_BASED_TEST(CanForwardRemoteAccount) {
		// Arrange:
		TestContext context;
		auto publicKey = test::GenerateRandomByteArray<Key>();
		auto remoteKey = test::GenerateRandomByteArray<Key>();
		context.addAccount(publicKey, state::AccountType::Main, remoteKey);
		context.addAccount(remoteKey, state::AccountType::Remote, publicKey);

		// Act:
		ProcessForwardedAccountState(TTraits::GetCache(context), remoteKey, [&publicKey](const auto& accountState) {
			// Assert: main account is returned
			EXPECT_EQ(publicKey, accountState.PublicKey);
		});
	}

	// endregion

	// region forward failure

	CACHE_BASED_TEST(CannotForwardWhenMainAccountIsNotPresent) {
		// Arrange:
		TestContext context;
		auto publicKey = test::GenerateRandomByteArray<Key>();
		auto remoteKey = test::GenerateRandomByteArray<Key>();
		context.addAccount(remoteKey, state::AccountType::Remote, publicKey);

		// Act + Assert:
		EXPECT_THROW(ProcessForwardedAccountState(TTraits::GetCache(context), remoteKey, [](const auto&) {}), catapult_invalid_argument);
	}

	CACHE_BASED_TEST(CannotForwardWhenMainHasInvalidAccountType) {
		// Arrange:
		TestContext context;
		auto publicKey = test::GenerateRandomByteArray<Key>();
		auto remoteKey = test::GenerateRandomByteArray<Key>();
		context.addAccount(publicKey, state::AccountType::Remote, remoteKey);
		context.addAccount(remoteKey, state::AccountType::Remote, publicKey);

		// Act + Assert:
		EXPECT_THROW(ProcessForwardedAccountState(TTraits::GetCache(context), remoteKey, [](const auto&) {}), catapult_runtime_error);
	}

	CACHE_BASED_TEST(CannotForwardWhenMainHasInvalidKey) {
		// Arrange: main account does not link-back to remote key
		TestContext context;
		auto publicKey = test::GenerateRandomByteArray<Key>();
		auto remoteKey = test::GenerateRandomByteArray<Key>();
		context.addAccount(publicKey, state::AccountType::Main, test::GenerateRandomByteArray<Key>());
		context.addAccount(remoteKey, state::AccountType::Remote, publicKey);

		// Act + Assert:
		EXPECT_THROW(ProcessForwardedAccountState(TTraits::GetCache(context), remoteKey, [](const auto&) {}), catapult_runtime_error);
	}

	// endregion
}}
