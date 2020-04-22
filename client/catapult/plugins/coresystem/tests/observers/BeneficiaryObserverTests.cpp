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
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/AccountObserverTestContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS BeneficiaryObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(Beneficiary,)

	namespace {
		constexpr auto Notification_Height = Height(100);
		constexpr auto Importance_Height = model::ImportanceHeight(98);
		constexpr auto Default_Beneficiary_Count = 200u;

		void SeedAccount(state::AccountState& accountState) {
			accountState.ActivityBuckets.update(Importance_Height, [](auto& bucket){
				bucket.BeneficiaryCount = Default_Beneficiary_Count;
				bucket.TotalFeesPaid = Amount(1000);
			});
		}

		class TestContext : public test::AccountObserverTestContext {
		public:
			explicit TestContext(NotifyMode notifyMode)
					: test::AccountObserverTestContext(notifyMode, Notification_Height, CreateBlockChainConfiguration())
			{}

		public:
			auto addAccount(const Key& publicKey) {
				auto& accountStateCache = cache().sub<cache::AccountStateCache>();
				accountStateCache.addAccount(publicKey, Height(123));
				return accountStateCache.find(publicKey);
			}

			auto setupRemote(const Key& publicKey, const Key& remoteKey) {
				auto remoteAccountStateIter = addAccount(remoteKey);
				remoteAccountStateIter.get().AccountType = state::AccountType::Remote;
				remoteAccountStateIter.get().SupplementalAccountKeys.linkedPublicKey().set(publicKey);

				auto& accountStateCache = cache().sub<cache::AccountStateCache>();
				auto accountStateIter = accountStateCache.find(publicKey);
				accountStateIter.get().AccountType = state::AccountType::Main;
				accountStateIter.get().SupplementalAccountKeys.linkedPublicKey().set(remoteKey);

				return remoteAccountStateIter;
			}

		private:
			static model::BlockChainConfiguration CreateBlockChainConfiguration() {
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.ImportanceGrouping = 2;
				return config;
			}
		};

		template<typename TAction>
		void RunBeneficiaryObserverTest(NotifyMode notifyMode, TAction action) {
			// Arrange:
			TestContext context(notifyMode);

			auto pObserver = CreateBeneficiaryObserver();

			// Act + Assert:
			action(context, *pObserver);
		}

		void AssertActivityUnset(const state::AccountState& accountState) {
			const auto& activityBucket = accountState.ActivityBuckets.get(Importance_Height);
			EXPECT_EQ(model::ImportanceHeight(), activityBucket.StartHeight);
			EXPECT_EQ(Amount(), activityBucket.TotalFeesPaid);
			EXPECT_EQ(0u, activityBucket.BeneficiaryCount);
		}

		void AssertActivitySet(const state::AccountState& accountState, uint32_t beneficiaryCount) {
			const auto& activityBucket = accountState.ActivityBuckets.get(Importance_Height);
			EXPECT_EQ(Importance_Height, activityBucket.StartHeight);
			EXPECT_EQ(Amount(1000), activityBucket.TotalFeesPaid);
			EXPECT_EQ(beneficiaryCount, activityBucket.BeneficiaryCount);
		}

		struct CommitTraits {
			static constexpr auto Notify_Mode = NotifyMode::Commit;

			static constexpr auto Expected_Count = Default_Beneficiary_Count + 1;
		};

		struct RollbackTraits {
			static constexpr auto Notify_Mode = NotifyMode::Rollback;

			static constexpr auto Expected_Count = Default_Beneficiary_Count - 1;
		};
	}

#define BENEFICIARY_OBSERVER_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Commit) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CommitTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Rollback) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RollbackTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region unique beneficiary

	BENEFICIARY_OBSERVER_TRAITS_BASED_TEST(BeneficiaryAccountIsUpdatedButSignerAccountIsNotUpdated) {
		// Arrange:
		RunBeneficiaryObserverTest(TTraits::Notify_Mode, [](auto& context, auto& observer) {
			auto signerPublicKey = test::GenerateRandomByteArray<Key>();
			auto beneficiaryPublicKey = test::GenerateRandomByteArray<Key>();
			auto signerAccountStateIter = context.addAccount(signerPublicKey);
			auto beneficiaryAccountStateIter = context.addAccount(beneficiaryPublicKey);
			SeedAccount(beneficiaryAccountStateIter.get());

			auto notification = test::CreateBlockNotification(signerPublicKey, beneficiaryPublicKey);
			notification.TotalFee = Amount(123);

			// Sanity:
			AssertActivityUnset(signerAccountStateIter.get());
			AssertActivitySet(beneficiaryAccountStateIter.get(), Default_Beneficiary_Count);

			// Act:
			test::ObserveNotification(observer, notification, context);

			// Assert:
			AssertActivityUnset(signerAccountStateIter.get());
			AssertActivitySet(beneficiaryAccountStateIter.get(), TTraits::Expected_Count);
		});
	}

	BENEFICIARY_OBSERVER_TRAITS_BASED_TEST(BeneficiaryAccountIsUpdatedWhenUsingRemote) {
		// Arrange:
		RunBeneficiaryObserverTest(TTraits::Notify_Mode, [](auto& context, auto& observer) {
			auto signerPublicKey = test::GenerateRandomByteArray<Key>();
			auto beneficiaryPublicKey = test::GenerateRandomByteArray<Key>();
			auto beneficiaryRemoteKey = test::GenerateRandomByteArray<Key>();
			auto signerAccountStateIter = context.addAccount(signerPublicKey);
			auto beneficiaryAccountStateIter = context.addAccount(beneficiaryPublicKey);
			auto beneficiaryRemoteStateIter = context.setupRemote(beneficiaryPublicKey, beneficiaryRemoteKey);
			SeedAccount(beneficiaryAccountStateIter.get());

			auto notification = test::CreateBlockNotification(signerPublicKey, beneficiaryRemoteKey);
			notification.TotalFee = Amount(123);

			// Sanity:
			AssertActivityUnset(signerAccountStateIter.get());
			AssertActivitySet(beneficiaryAccountStateIter.get(), Default_Beneficiary_Count);
			AssertActivityUnset(beneficiaryRemoteStateIter.get());

			// Act:
			test::ObserveNotification(observer, notification, context);

			// Assert:
			AssertActivityUnset(signerAccountStateIter.get());
			AssertActivitySet(beneficiaryAccountStateIter.get(), TTraits::Expected_Count);
			AssertActivityUnset(beneficiaryRemoteStateIter.get());
		});
	}

	// endregion

	// region self-beneficiary

	BENEFICIARY_OBSERVER_TRAITS_BASED_TEST(BeneficiaryAccountIsUpdatedWhenSameAsSigner) {
		// Arrange:
		RunBeneficiaryObserverTest(TTraits::Notify_Mode, [](auto& context, auto& observer) {
			auto signerPublicKey = test::GenerateRandomByteArray<Key>();
			auto signerAccountStateIter = context.addAccount(signerPublicKey);
			SeedAccount(signerAccountStateIter.get());

			auto notification = test::CreateBlockNotification(signerPublicKey, signerPublicKey);
			notification.TotalFee = Amount(123);

			// Sanity:
			AssertActivitySet(signerAccountStateIter.get(), Default_Beneficiary_Count);

			// Act:
			test::ObserveNotification(observer, notification, context);

			// Assert:
			AssertActivitySet(signerAccountStateIter.get(), TTraits::Expected_Count);
		});
	}

	// endregion
}}
