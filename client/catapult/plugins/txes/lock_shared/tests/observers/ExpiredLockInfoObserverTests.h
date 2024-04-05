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

#pragma once
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

	/// Provides scaffolding for expired lock info observer tests.
	template<typename TTraits>
	class ExpiredLockInfoObserverTests {
	public:
		enum class HarvesterType { Main, Remote };

		struct SeedTuple {
			Address OwnerAddress;
			catapult::MosaicId MosaicId;
			Amount InitialBalance;
			Amount LockAmount;
		};

		struct BalanceTestOptions {
		public:
			bool ExpectTouches;
			bool SetFutureExpiration;

		public:
			BalanceTestOptions() {
				ExpectTouches = true;
				SetFutureExpiration = false;
			}
		};

	public:
		// region RunBalanceTest / RunReceiptTest

		static void RunBalanceTest(
				NotifyMode mode,
				HarvesterType harvesterType,
				const Key& harvesterPublicKey,
				const std::vector<SeedTuple>& expiringSeeds,
				const std::vector<SeedTuple>& expectedPostObserveBalances,
				const BalanceTestOptions& options = BalanceTestOptions()) {
			// Arrange:
			TestContext context(Height(55), mode);
			auto notificationBlockHarvester = context.addBlockHarvester(harvesterType, harvesterPublicKey, MosaicId(500), Amount(200));

			// - add placeholder accounts that expire at other heights: 10, 20, ..., 100
			auto seeds = GenerateSeeds(10);
			context.addLockInfos(seeds, [](auto i) { return Height((i + 1) * 10); });

			// - add expiring accounts
			context.addLockInfos(expiringSeeds, [](auto) { return Height(55); }, options.SetFutureExpiration);

			// Act:
			context.observe(notificationBlockHarvester);

			// Assert: all expiring lock infos were touched (options.ExpectTouches, default)
			//         or NONE were touched (!options.ExpectTouches)
			context.assertLockInfoTouches(options.ExpectTouches ? expiringSeeds.size() : 0);

			// - unaffected accounts have unchanged balances
			context.assertBalances(seeds, "unaffected accounts");

			// - potentially affected accounts have expected balances
			context.assertBalances(expectedPostObserveBalances, "potentially affected accounts");
		}

		static void RunReceiptTest(
				NotifyMode mode,
				HarvesterType harvesterType,
				const Key& harvesterPublicKey,
				const std::vector<SeedTuple>& expiringSeeds,
				const std::vector<SeedTuple>& expectedReceipts) {
			// Arrange:
			TestContext context(Height(55), mode);
			auto notificationBlockHarvester = context.addBlockHarvester(harvesterType, harvesterPublicKey, MosaicId(500), Amount(200));

			// - add placeholder accounts that expire at other heights: 10, 20, ..., 100
			auto seeds = GenerateSeeds(10);
			context.addLockInfos(seeds, [](auto i) { return Height((i + 1) * 10); });

			// - add expiring accounts
			context.addLockInfos(expiringSeeds, [](auto) { return Height(55); });

			// Act:
			auto pStatement = context.observe(notificationBlockHarvester);

			// Assert:
			if (expectedReceipts.empty()) {
				EXPECT_EQ(0u, pStatement->TransactionStatements.size());
				return;
			}

			ASSERT_EQ(1u, pStatement->TransactionStatements.size());

			const auto& receiptPair = *pStatement->TransactionStatements.find(model::ReceiptSource());
			ASSERT_EQ(expectedReceipts.size(), receiptPair.second.size());

			auto i = 0u;
			for (const auto& expectedReceipt : expectedReceipts) {
				auto message = "receipt at " + std::to_string(i);
				const auto& receipt = static_cast<const model::BalanceChangeReceipt&>(receiptPair.second.receiptAt(i));

				ASSERT_EQ(sizeof(model::BalanceChangeReceipt), receipt.Size) << message;
				EXPECT_EQ(1u, receipt.Version) << message;
				EXPECT_EQ(TTraits::Receipt_Type, receipt.Type) << message;
				EXPECT_EQ(expectedReceipt.MosaicId, receipt.Mosaic.MosaicId) << message;
				EXPECT_EQ(expectedReceipt.LockAmount, receipt.Mosaic.Amount) << message;
				EXPECT_EQ(expectedReceipt.OwnerAddress, receipt.TargetAddress) << message;
				++i;
			}
		}

		// endregion

	private:
		// region test utils

		static std::vector<SeedTuple> GenerateSeeds(size_t count) {
			std::vector<SeedTuple> seeds;
			for (auto i = 0u; i < count; ++i)
				seeds.push_back({ test::GenerateRandomByteArray<Address>(), MosaicId(2 * (i + 1)), Amount(100 + i), Amount(i) });

			return seeds;
		}

		// endregion

	private:
		// region TestContext

		class TestContext {
		private:
			using ObserverTestContext = typename TTraits::ObserverTestContext;
			using HeightGenerator = std::function<Height (uint32_t)>;

		public:
			TestContext(Height height, NotifyMode mode)
					: m_height(height)
					, m_mode(mode)
					, m_observerContext(m_mode, m_height)
			{}

		public:
			void addLockInfos(
					const std::vector<SeedTuple>& seeds,
					const HeightGenerator& heightGenerator,
					bool setFutureExpiration = false) {
				for (auto i = 0u; i < seeds.size(); ++i)
					addLockInfo(seeds[i], heightGenerator(i), setFutureExpiration);
			}

			Address addBlockHarvester(HarvesterType harvesterType, const Key& harvesterPublicKey, MosaicId mosaicId, Amount amount) {
				auto& accountStateCache = this->accountStateCache();
				accountStateCache.addAccount(harvesterPublicKey, Height(1));

				auto harvesterAccountStateIter = accountStateCache.find(harvesterPublicKey);
				harvesterAccountStateIter.get().Balances.credit(mosaicId, amount);

				if (HarvesterType::Main == harvesterType)
					return harvesterAccountStateIter.get().Address;

				// HarvesterType::Remote - interpret harvesterPublicKey as a main account and link it to a remote account
				auto remotePublicKey = test::GenerateRandomByteArray<Key>();
				accountStateCache.addAccount(remotePublicKey, Height(1));

				harvesterAccountStateIter.get().AccountType = state::AccountType::Main;
				harvesterAccountStateIter.get().SupplementalPublicKeys.linked().set(remotePublicKey);

				auto remoteAccountStateIter = accountStateCache.find(remotePublicKey);
				remoteAccountStateIter.get().AccountType = state::AccountType::Remote;
				remoteAccountStateIter.get().SupplementalPublicKeys.linked().set(harvesterPublicKey);
				return remoteAccountStateIter.get().Address;
			}

			void addLockInfo(const SeedTuple& seed, Height height, bool setFutureExpiration) {
				// lock info cache
				auto& lockInfoCacheDelta = TTraits::SubCache(m_observerContext.cache());
				auto lockInfo = TTraits::CreateLockInfo(height);
				lockInfo.OwnerAddress = seed.OwnerAddress;
				lockInfo.MosaicId = seed.MosaicId;
				lockInfo.Amount = seed.LockAmount;
				lockInfoCacheDelta.insert(lockInfo);

				if (setFutureExpiration) {
					lockInfo.EndHeight = Height(lockInfo.EndHeight.unwrap() + 10);
					lockInfoCacheDelta.insert(lockInfo);
				}

				// account state cache
				auto& accountStateCache = this->accountStateCache();
				if (!accountStateCache.contains(seed.OwnerAddress)) {
					accountStateCache.addAccount(seed.OwnerAddress, Height(1));
					accountStateCache.find(seed.OwnerAddress).get().Balances.credit(seed.MosaicId, seed.InitialBalance);
				}
			}

		public:
			auto observe(const Address& blockHarvester) {
				// Arrange:
				auto pObserver = TTraits::CreateObserver();
				auto notification = test::CreateBlockNotification(blockHarvester);

				// - commit all cache changes in order to detect changes triggered by observe
				m_observerContext.commitCacheChanges();

				// Act:
				test::ObserveNotification(*pObserver, notification, m_observerContext);
				return m_observerContext.statementBuilder().build();
			}

		public:
			void assertLockInfoTouches(size_t numExpiringLockInfos) {
				auto& lockInfoCacheDelta = TTraits::SubCache(m_observerContext.cache());
				EXPECT_TRUE(lockInfoCacheDelta.addedElements().empty());
				EXPECT_EQ(numExpiringLockInfos, lockInfoCacheDelta.modifiedElements().size());
				EXPECT_TRUE(lockInfoCacheDelta.removedElements().empty());
			}

			void assertBalances(const std::vector<SeedTuple>& seeds, const std::string& message) {
				auto i = 0u;
				auto& accountStateCache = this->accountStateCache();
				for (const auto& seed : seeds) {
					const auto& balances = accountStateCache.find(seed.OwnerAddress).get().Balances;
					EXPECT_EQ(1u, balances.size()) << message << " at " << i;
					EXPECT_EQ(seed.InitialBalance, balances.get(seed.MosaicId)) << message << " at " << i;
					++i;
				}
			}

		private:
			cache::AccountStateCacheDelta& accountStateCache() {
				return m_observerContext.cache().template sub<cache::AccountStateCache>();
			}

		private:
			Height m_height;
			NotifyMode m_mode;
			ObserverTestContext m_observerContext;
		};

		// endregion
	};
}}
