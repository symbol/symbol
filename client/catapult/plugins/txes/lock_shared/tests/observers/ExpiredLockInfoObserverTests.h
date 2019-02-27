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

#pragma once
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/utils/ArraySet.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

	template<typename TTraits>
	struct ExpiredLockInfoObserverTests {
	public:
		static constexpr auto Initial_Balance = Amount(500);
		static constexpr auto Lock_Mosaic_Id = MosaicId(1234);
		static constexpr auto Lock_Amount = Amount(100);

		using HeightGenerator = std::function<Height (uint32_t)>;

		// region no operation

		static void AssertObserverDoesNothingWhenNoLockInfoExpired_OnCommit() {
			// Assert:
			AssertObserverDoesNothingWhenNoLockInfoExpired(NotifyMode::Commit);
		}

		static void AssertObserverDoesNothingWhenNoLockInfoExpired_OnRollback() {
			// Assert:
			AssertObserverDoesNothingWhenNoLockInfoExpired(NotifyMode::Rollback);
		}

		// endregion

		// region expiration

		static void AssertObserverCreditsAccountsOnCommit_Single() {
			// Assert:
			AssertObserverTransfersMosaics(NotifyMode::Commit, 1);
		}

		static void AssertObserverCreditsAccountsOnCommit_Multiple() {
			// Assert:
			AssertObserverTransfersMosaics(NotifyMode::Commit, 3);
		}

		static void AssertObserverCreditsAccountsOnRollback_Single() {
			// Assert:
			AssertObserverTransfersMosaics(NotifyMode::Rollback, 1);
		}

		static void AssertObserverCreditsAccountsOnRollback_Multiple() {
			// Assert:
			AssertObserverTransfersMosaics(NotifyMode::Rollback, 3);
		}

		// endregion

	private:
		class TestContext {
		private:
			using ObserverTestContext = typename TTraits::ObserverTestContext;

		public:
			TestContext(Height height, NotifyMode mode, const model::Mosaic& seedMosaic)
					: m_height(height)
					, m_mode(mode)
					, m_seedMosaic(seedMosaic)
					, m_observerContext(m_mode, m_height)
			{}

		public:
			utils::KeySet addLockInfos(size_t numLockInfos, const HeightGenerator& heightGenerator) {
				// Arrange: seed caches
				utils::KeySet keys;
				auto& lockInfoCacheDelta = TTraits::SubCache(m_observerContext.cache());
				auto& accountStateCache = this->accountStateCache();
				for (auto i = 0u; i < numLockInfos; ++i) {
					// - lock info cache
					auto lockInfo = CreateLockInfoWithAmount(Lock_Amount, heightGenerator(i));
					keys.insert(lockInfo.Account);
					lockInfoCacheDelta.insert(lockInfo);

					// - account state cache
					accountStateCache.addAccount(lockInfo.Account, Height(1));
					credit(accountStateCache.find(lockInfo.Account).get());
				}

				return keys;
			}

			Key addBlockSigner() {
				// Arrange: add block signer to account state cache
				auto& accountStateCache = this->accountStateCache();
				auto signer = test::GenerateRandomData<Key_Size>();
				accountStateCache.addAccount(signer, Height(1));
				credit(accountStateCache.find(signer).get());
				return signer;
			}

		public:
			cache::CatapultCacheDelta& observe(const Key& blockSigner) {
				// Arrange:
				auto pObserver = TTraits::CreateObserver();
				auto notification = test::CreateBlockNotification(blockSigner);

				// - commit all cache changes in order to detect changes triggered by observe
				m_observerContext.commitCacheChanges();

				// Act:
				test::ObserveNotification(*pObserver, notification, m_observerContext);
				return m_observerContext.cache();
			}

		public:
			void assertNoLockInfoChanges() {
				auto& lockInfoCacheDelta = TTraits::SubCache(m_observerContext.cache());
				EXPECT_TRUE(lockInfoCacheDelta.addedElements().empty());
				EXPECT_TRUE(lockInfoCacheDelta.modifiedElements().empty());
				EXPECT_TRUE(lockInfoCacheDelta.removedElements().empty());
			}

			void assertBalances(const utils::KeySet& accountIds, const model::Mosaic& expectedMosaic, const std::string& message) {
				auto& accountStateCache = this->accountStateCache();
				for (const auto& accountId : accountIds) {
					const auto& balances = accountStateCache.find(accountId).get().Balances;
					EXPECT_EQ(1u, balances.size()) << message;
					EXPECT_EQ(expectedMosaic.Amount, balances.get(expectedMosaic.MosaicId)) << message;
				}
			}

		private:
			cache::AccountStateCacheDelta& accountStateCache() {
				return m_observerContext.cache().template sub<cache::AccountStateCache>();
			}

			void credit(state::AccountState& accountState) {
				accountState.Balances.credit(m_seedMosaic.MosaicId, m_seedMosaic.Amount);
			}

		private:
			static typename TTraits::ValueType CreateLockInfoWithAmount(Amount amount, Height height) {
				auto lockInfo = TTraits::CreateLockInfo(height);
				lockInfo.MosaicId = Lock_Mosaic_Id;
				lockInfo.Amount = amount;
				return lockInfo;
			}

		private:
			Height m_height;
			NotifyMode m_mode;
			model::Mosaic m_seedMosaic;
			ObserverTestContext m_observerContext;
		};

	private:
		static void AssertObserverDoesNothingWhenNoLockInfoExpired(NotifyMode mode) {
			// Arrange:
			auto seedMosaic = model::Mosaic{ Lock_Mosaic_Id, Initial_Balance };
			TestContext context(Height(55), mode, seedMosaic);
			auto blockSigner = context.addBlockSigner();

			// - expiry heights are 10, 20, ..., 100
			auto lockInfoKeys = context.addLockInfos(10, [](auto i) { return Height((i + 1) * 10); });

			// Act:
			context.observe(blockSigner);

			// Assert: lock info cache didn't change
			context.assertNoLockInfoChanges();

			// - owner accounts of lock infos were not credited / debited mosaics
			context.assertBalances(lockInfoKeys, seedMosaic, "lockInfoKeys");

			// - block signer was not credited / debited mosaics
			context.assertBalances({ blockSigner }, seedMosaic, "blockSigner");
		}

		static void AssertObserverTransfersMosaics(NotifyMode mode, size_t numExpiringLockInfos) {
			// Arrange:
			auto seedMosaic = model::Mosaic{ Lock_Mosaic_Id, Initial_Balance };
			TestContext context(Height(55), mode, seedMosaic);
			auto blockSigner = context.addBlockSigner();

			// - expiry heights are 10, 20, ..., 100
			auto lockInfoKeys = context.addLockInfos(10, [](auto i) { return Height((i + 1) * 10); });

			// - expiry heights at 55
			auto expiringLockInfoKeys = context.addLockInfos(numExpiringLockInfos, [](auto) { return Height(55); });

			// Act:
			context.observe(blockSigner);

			// Assert: lock info cache didn't change
			context.assertNoLockInfoChanges();

			// - owner accounts of lock infos not expiring at height were not credited / debited mosaics
			context.assertBalances(lockInfoKeys, seedMosaic, "lockInfoKeys");

			// - owner accounts of lock infos expiring at height might have been credited / debited mosaics
			auto expectedExpiringMosaic = model::Mosaic{
				seedMosaic.MosaicId,
				TTraits::GetExpectedExpiringLockOwnerBalance(mode, Initial_Balance, Lock_Amount)
			};
			context.assertBalances(expiringLockInfoKeys, expectedExpiringMosaic, "expiringLockInfoKeys");

			// - block signer might have been credited / debited mosaics
			auto expectedSignerMosaic = model::Mosaic{
				seedMosaic.MosaicId,
				TTraits::GetExpectedBlockSignerBalance(mode, Initial_Balance, Lock_Amount, numExpiringLockInfos)
			};
			context.assertBalances({ blockSigner }, expectedSignerMosaic, "blockSigner");
		}
	};

#define MAKE_EXPIRED_LOCK_INFO_OBSERVER_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { ExpiredLockInfoObserverTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_EXPIRED_LOCK_INFO_OBSERVER_TESTS(TRAITS_NAME) \
	MAKE_EXPIRED_LOCK_INFO_OBSERVER_TEST(TRAITS_NAME, ObserverDoesNothingWhenNoLockInfoExpired_OnCommit) \
	MAKE_EXPIRED_LOCK_INFO_OBSERVER_TEST(TRAITS_NAME, ObserverDoesNothingWhenNoLockInfoExpired_OnRollback) \
	MAKE_EXPIRED_LOCK_INFO_OBSERVER_TEST(TRAITS_NAME, ObserverCreditsAccountsOnCommit_Single) \
	MAKE_EXPIRED_LOCK_INFO_OBSERVER_TEST(TRAITS_NAME, ObserverCreditsAccountsOnCommit_Multiple) \
	MAKE_EXPIRED_LOCK_INFO_OBSERVER_TEST(TRAITS_NAME, ObserverCreditsAccountsOnRollback_Single) \
	MAKE_EXPIRED_LOCK_INFO_OBSERVER_TEST(TRAITS_NAME, ObserverCreditsAccountsOnRollback_Multiple)
}}
