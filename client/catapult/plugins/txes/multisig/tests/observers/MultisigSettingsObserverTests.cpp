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
#include "tests/test/MultisigCacheTestUtils.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS MultisigSettingsObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(MultisigSettings,)

	namespace {
		using ObserverTestContext = test::ObserverTestContextT<test::MultisigCacheFactory>;
		using Notification = model::MultisigSettingsNotification;

		auto CreateNotification(const Address& multisig, int8_t minRemovalDelta, int8_t minApprovalDelta) {
			return Notification(multisig, minRemovalDelta, minApprovalDelta);
		}

		struct MultisigSettings {
		public:
			uint32_t Removal;
			uint32_t Approval;
		};

		struct TestSettings {
		public:
			uint32_t Expected;
			uint32_t Current;
			int8_t Delta;
		};

		void RunTest(
				ObserverTestContext&& context,
				const Address& multisig,
				const MultisigSettings& initialSettings,
				const Notification& notification,
				const MultisigSettings& expectedSettings) {
			// Arrange:
			auto pObserver = CreateMultisigSettingsObserver();

			// - seed the cache
			auto& multisigCacheDelta = context.cache().sub<cache::MultisigCache>();

			// - create multisig entry in cache
			{
				multisigCacheDelta.insert(state::MultisigEntry(multisig));
				auto& entry = multisigCacheDelta.find(multisig).get();
				entry.setMinRemoval(initialSettings.Removal);
				entry.setMinApproval(initialSettings.Approval);
			}

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert: check the cache
			const auto& entry = multisigCacheDelta.find(multisig).get();
			EXPECT_EQ(expectedSettings.Removal, entry.minRemoval()) << "initial: " << static_cast<int>(initialSettings.Removal);
			EXPECT_EQ(expectedSettings.Approval, entry.minApproval()) << "initial: " << static_cast<int>(initialSettings.Approval);
		}

		struct CommitTraits {
		public:
			static constexpr NotifyMode Mode = NotifyMode::Commit;

			static void AssertTestWithSettings(const TestSettings& removal, const TestSettings& approval) {
				// Arrange:
				auto multisig = test::GenerateRandomByteArray<Address>();
				auto notification = CreateNotification(multisig, removal.Delta, approval.Delta);

				// Act + Assert:
				RunTest(
						ObserverTestContext(Mode, Height(777)),
						multisig,
						{ removal.Current, approval.Current },
						notification,
						{ removal.Expected, approval.Expected });
			}
		};

		struct RollbackTraits {
		public:
			static constexpr NotifyMode Mode = NotifyMode::Rollback;

			static void AssertTestWithSettings(const TestSettings& removal, const TestSettings& approval) {
				// Arrange:
				auto multisig = test::GenerateRandomByteArray<Address>();
				auto notification = CreateNotification(multisig, removal.Delta, approval.Delta);

				// Act + Assert:
				RunTest(
						ObserverTestContext(Mode, Height(777)),
						multisig,
						{ removal.Expected, approval.Expected },
						notification,
						{ removal.Current, approval.Current });
			}
		};
	}

	// region traits based tests

#define NOTIFY_MODE_BASED_TRAITS(TEST_NAME) \
	template<typename TTraits> void TEST_NAME(); \
	TEST(TEST_CLASS, TEST_NAME##_Commit) { TEST_NAME<CommitTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Rollback) { TEST_NAME<RollbackTraits>(); } \
	template<typename TTraits> void TEST_NAME()

	NOTIFY_MODE_BASED_TRAITS(ZeroDeltaDoesNotChangeSettings) {
		// Assert:
		TTraits::AssertTestWithSettings({ 10, 10, 0 }, { 123, 123, 0 });
	}

	NOTIFY_MODE_BASED_TRAITS(PositiveDeltaIncreasesSettings) {
		// Assert:
		TTraits::AssertTestWithSettings({ 22, 10, 12, }, { 157, 123, 34 });
	}

	NOTIFY_MODE_BASED_TRAITS(NegativeDeltaDecreasesSettings) {
		// Assert:
		TTraits::AssertTestWithSettings({ 7, 10, -3 }, { 89, 123, -34 });
	}

	NOTIFY_MODE_BASED_TRAITS(ObserverDoesNotCareAboutWrapAround) {
		// Assert: this is checked by settings validator
		auto maxUnsignedValue = std::numeric_limits<uint32_t>::max();
		TTraits::AssertTestWithSettings({ 124, maxUnsignedValue - 1, 126 }, { maxUnsignedValue - 127, 0, -128 });
	}

	// endregion

	// region notify mode specific tests

	TEST(TEST_CLASS, ObserverIgnoresNotificationWhenAccountIsUnknown_ModeCommit) {
		// Arrange:
		auto multisig = test::GenerateRandomByteArray<Address>();
		auto notification = CreateNotification(multisig, -1, -2);

		auto pObserver = CreateMultisigSettingsObserver();
		ObserverTestContext context(NotifyMode::Commit, Height(777));

		// Act: observer does not throw
		test::ObserveNotification(*pObserver, notification, context);

		// Assert: cache was not altered
		const auto& multisigCache = context.cache().sub<cache::MultisigCache>();
		EXPECT_EQ(0u, multisigCache.size());
	}

	TEST(TEST_CLASS, ObserverAddsUnknownAccountToCacheAndProcessesDeltas_ModeRollback) {
		// Arrange:
		auto multisig = test::GenerateRandomByteArray<Address>();
		auto notification = CreateNotification(multisig, -1, -2);

		auto pObserver = CreateMultisigSettingsObserver();
		ObserverTestContext context(NotifyMode::Rollback, Height(777));

		// Act:
		test::ObserveNotification(*pObserver, notification, context);

		// Assert:
		const auto& multisigCache = context.cache().sub<cache::MultisigCache>();
		EXPECT_EQ(1u, multisigCache.size());

		auto multisigIter = multisigCache.find(multisig);
		ASSERT_TRUE(!!multisigIter.tryGet());

		const auto& multisigEntry = multisigIter.get();
		EXPECT_EQ(multisig, multisigEntry.address());
		EXPECT_EQ(2u, multisigEntry.minApproval());
		EXPECT_EQ(1u, multisigEntry.minRemoval());
		EXPECT_TRUE(multisigEntry.cosignatoryAddresses().empty());
		EXPECT_TRUE(multisigEntry.multisigAddresses().empty());
	}

	// endregion
}}
