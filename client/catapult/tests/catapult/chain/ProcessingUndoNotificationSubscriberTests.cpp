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

#include "catapult/chain/ProcessingUndoNotificationSubscriber.h"
#include "catapult/cache/CatapultCache.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/test/other/mocks/MockNotificationObserver.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS ProcessingUndoNotificationSubscriberTests

	namespace {
		constexpr auto MakeTestNotificationType(model::NotificationChannel channel, uint16_t code = 1) {
			return model::MakeNotificationType(channel, static_cast<model::FacilityCode>(0), code);
		}

		constexpr auto Notification_Type_Validator = MakeTestNotificationType(model::NotificationChannel::Validator);
		constexpr auto Notification_Type_Observer = MakeTestNotificationType(model::NotificationChannel::Observer);
		constexpr auto Notification_Type_All = MakeTestNotificationType(model::NotificationChannel::All);
		constexpr auto Notification_Type_All_2 = MakeTestNotificationType(model::NotificationChannel::All, 2);
		constexpr auto Notification_Type_All_3 = MakeTestNotificationType(model::NotificationChannel::All, 3);

		constexpr auto CreateResolverContext = test::CreateResolverContextWithCustomDoublingMosaicResolver;

		class TestContext {
		public:
			explicit TestContext(observers::NotifyMode executeMode = observers::NotifyMode::Commit)
					: m_cache({})
					, m_cacheDelta(m_cache.createDelta())
					, m_observerContext(
							model::NotificationContext(Height(123), CreateResolverContext()),
							observers::ObserverState(m_cacheDelta),
							executeMode)
					, m_sub(m_observer, m_observerContext) {
				CATAPULT_LOG(debug) << "preparing test context with execute mode " << executeMode;
			}

		public:
			ProcessingUndoNotificationSubscriber& sub() {
				return m_sub;
			}

		public:
			void assertUndoObserverCalls(const std::vector<model::NotificationType>& expectedTypes) {
				// Assert:
				ASSERT_EQ(expectedTypes.size(), m_observer.notificationTypes().size());

				for (auto i = 0u; i < expectedTypes.size(); ++i) {
					auto message = "observer notification at " + std::to_string(i);
					EXPECT_EQ(expectedTypes[i], m_observer.notificationTypes()[i]) << message;

					// - cache should refer to same object
					const auto& observerContext = m_observer.contexts()[i];
					EXPECT_EQ(&m_observerContext.Cache, &observerContext.Cache) << message;

					// - height should be the same but mode should be reversed
					EXPECT_EQ(m_observerContext.Height, observerContext.Height) << message;
					auto expectedUndoMode = observers::NotifyMode::Commit == m_observerContext.Mode
							? observers::NotifyMode::Rollback
							: observers::NotifyMode::Commit;
					EXPECT_EQ(expectedUndoMode, observerContext.Mode) << message;

					// - appropriate resolvers were passed down
					EXPECT_EQ(MosaicId(22), observerContext.Resolvers.resolve(UnresolvedMosaicId(11)));
				}
			}

			void assertUndoObserverHashes(const std::vector<Hash256>& expectedHashes) {
				// Assert:
				ASSERT_EQ(expectedHashes.size(), m_observer.notificationTypes().size());

				for (auto i = 0u; i < expectedHashes.size(); ++i)
					EXPECT_EQ(expectedHashes[i], m_observer.notificationHashes()[i]) << "observer notification hash at " << i;
			}

		private:
			mocks::MockNotificationObserver m_observer;

			cache::CatapultCache m_cache;
			cache::CatapultCacheDelta m_cacheDelta;

			observers::ObserverContext m_observerContext;

			ProcessingUndoNotificationSubscriber m_sub;
		};
	}

	// region basic

	namespace {
		void AssertCannotProcessNotificationWithInvalidSize(size_t size) {
			// Arrange:
			TestContext context;
			auto notification = test::CreateNotification(Notification_Type_All);
			notification.Size = size;

			// Act + Assert:
			EXPECT_THROW(context.sub().notify(notification), catapult_invalid_argument) << "size: " << size;
		}
	}

	TEST(TEST_CLASS, CannotProcessNotificationWithInvalidSize) {
		AssertCannotProcessNotificationWithInvalidSize(0);
		AssertCannotProcessNotificationWithInvalidSize(sizeof(model::Notification) - 1);
	}

	TEST(TEST_CLASS, NotificationsHaveNoEffectUnlessUndone) {
		// Arrange:
		TestContext context;
		auto notification = test::CreateNotification(Notification_Type_All);

		// Act: process notification
		context.sub().notify(notification);

		// Assert: no calls
		context.assertUndoObserverCalls({});
	}

	// endregion

	// region basic fixed size undo (zero, single, multiple)

#define NOTIFY_MODE_TRAITS_BASED_TEST(TEST_NAME) \
	template<observers::NotifyMode TMode> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Commit) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<observers::NotifyMode::Commit>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Rollback) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<observers::NotifyMode::Rollback>(); } \
	template<observers::NotifyMode TMode> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	NOTIFY_MODE_TRAITS_BASED_TEST(CanUndoZeroNotifications) {
		// Arrange:
		TestContext context(TMode);

		// Act: undo notification
		context.sub().undo();

		// Assert:
		context.assertUndoObserverCalls({});
	}

	NOTIFY_MODE_TRAITS_BASED_TEST(CanUndoSingleNotification) {
		// Arrange:
		TestContext context(TMode);
		auto notification = test::CreateNotification(Notification_Type_All);

		// - process notification
		context.sub().notify(notification);

		// Act: undo notification
		context.sub().undo();

		// Assert:
		context.assertUndoObserverCalls({ Notification_Type_All });
	}

	NOTIFY_MODE_TRAITS_BASED_TEST(CanUndoMultipleNotifications) {
		// Arrange:
		TestContext context(TMode);
		auto notification1 = test::CreateNotification(Notification_Type_All);
		auto notification2 = test::CreateNotification(Notification_Type_All_2);
		auto notification3 = test::CreateNotification(Notification_Type_All_3);

		// - process notifications
		context.sub().notify(notification1);
		context.sub().notify(notification2);
		context.sub().notify(notification3);

		// Act: undo notification
		context.sub().undo();

		// Assert: notice that notifications are undone in reverse order
		context.assertUndoObserverCalls({ Notification_Type_All_3, Notification_Type_All_2, Notification_Type_All });
	}

	NOTIFY_MODE_TRAITS_BASED_TEST(OnlyObservableNotificationsCanBeUndone) {
		// Arrange:
		TestContext context(TMode);
		auto notification1 = test::CreateNotification(Notification_Type_Validator);
		auto notification2 = test::CreateNotification(Notification_Type_All_2);
		auto notification3 = test::CreateNotification(Notification_Type_Observer);

		// - process notifications
		context.sub().notify(notification1);
		context.sub().notify(notification2);
		context.sub().notify(notification3);

		// Act: undo notification
		context.sub().undo();

		// Assert: notice that notifications are undone in reverse order
		context.assertUndoObserverCalls({ Notification_Type_Observer, Notification_Type_All_2 });
	}

	// endregion

	namespace {
		Hash256 CalculateNotificationHash(const model::Notification& notification) {
			return test::CalculateNotificationHash(notification);
		}
	}

	TEST(TEST_CLASS, CanUndoMultipleNotificationsWithVaryingSizesAndChannels) {
		// Arrange:
		TestContext context;
		auto sender = test::GenerateRandomByteArray<Address>();
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto notification1 = model::AccountAddressNotification(sender);
		auto notification2 = test::CreateNotification(Notification_Type_All);
		auto notification3 = model::EntityNotification(model::NetworkIdentifier::Private_Test, 0, 0, 0);
		auto notification4 = model::TransactionNotification(sender, hash, static_cast<model::EntityType>(22), Timestamp(11));

		// - process notifications
		context.sub().notify(notification1);
		context.sub().notify(notification2);
		context.sub().notify(notification3);
		context.sub().notify(notification4);

		// Act: undo notification
		context.sub().undo();

		// Assert:
		// - notice that notifications are undone in reverse order
		// - notice that notification1 is observer-only
		// - notice that notification3 is validator-only
		context.assertUndoObserverCalls({
			model::Core_Transaction_Notification, Notification_Type_All, model::Core_Register_Account_Address_Notification
		});

		// - check data integrity
		context.assertUndoObserverHashes({
			CalculateNotificationHash(notification4), CalculateNotificationHash(notification2), CalculateNotificationHash(notification1)
		});
	}

	TEST(TEST_CLASS, UndoIsIdempotent) {
		// Arrange:
		TestContext context;
		auto notification = test::CreateNotification(Notification_Type_All);

		// - process notification
		context.sub().notify(notification);

		// Act: undo notification
		for (auto i = 0u; i < 5; ++i)
			context.sub().undo();

		// Assert:
		context.assertUndoObserverCalls({ Notification_Type_All });
	}
}}
