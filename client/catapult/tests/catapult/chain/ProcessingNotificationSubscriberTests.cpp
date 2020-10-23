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

#include "catapult/chain/ProcessingNotificationSubscriber.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/test/other/mocks/MockNotificationObserver.h"
#include "tests/test/other/mocks/MockNotificationValidator.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

using catapult::validators::ValidationResult;

namespace catapult { namespace chain {

#define TEST_CLASS ProcessingNotificationSubscriberTests

	namespace {
		constexpr auto MakeTestNotificationType(model::NotificationChannel channel, uint16_t code = 1) {
			return model::MakeNotificationType(channel, static_cast<model::FacilityCode>(0), code);
		}

		constexpr auto Notification_Type_None = MakeTestNotificationType(model::NotificationChannel::None);
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
					, m_validatorContext(test::CreateValidatorContext(Height(123), m_cacheDelta.toReadOnly()))
					, m_observerContext(
							model::NotificationContext(Height(123), CreateResolverContext()),
							observers::ObserverState(m_cacheDelta),
							executeMode)
					, m_sub(m_validator, m_validatorContext, m_observer, m_observerContext) {
				CATAPULT_LOG(debug) << "preparing test context with execute mode " << executeMode;
			}

		public:
			ProcessingNotificationSubscriber& sub() {
				return m_sub;
			}

			void setValidationResult(ValidationResult result) {
				m_validator.setResult(result);
			}

		public:
			void assertValidatorCalls(const std::vector<model::NotificationType>& expectedTypes) {
				// Assert:
				ASSERT_EQ(expectedTypes.size(), m_validator.notificationTypes().size());

				for (auto i = 0u; i < expectedTypes.size(); ++i) {
					auto message = "validator notification at " + std::to_string(i);
					EXPECT_EQ(expectedTypes[i], m_validator.notificationTypes()[i]) << message;
					EXPECT_EQ(&m_validatorContext, m_validator.contextPointers()[i]) << message;
				}
			}

			void assertObserverCalls(
					const std::vector<model::NotificationType>& expectedTypes,
					size_t undoStartIndex = std::numeric_limits<size_t>::max()) {
				// Assert:
				ASSERT_EQ(expectedTypes.size(), m_observer.notificationTypes().size());

				for (auto i = 0u; i < expectedTypes.size(); ++i) {
					auto message = "observer notification at " + std::to_string(i);
					EXPECT_EQ(expectedTypes[i], m_observer.notificationTypes()[i]) << message;
					if (i < undoStartIndex) {
						// - unmodified context should be used for execution
						EXPECT_EQ(&m_observerContext, m_observer.contextPointers()[i]) << message;
					} else {
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
			}

			void assertObserverHashes(const std::vector<Hash256>& expectedHashes) {
				// Assert:
				ASSERT_EQ(expectedHashes.size(), m_observer.notificationTypes().size());

				for (auto i = 0u; i < expectedHashes.size(); ++i)
					EXPECT_EQ(expectedHashes[i], m_observer.notificationHashes()[i]) << "observer notification hash at " << i;
			}

		private:
			mocks::MockNotificationValidator m_validator;
			mocks::MockNotificationObserver m_observer;

			cache::CatapultCache m_cache;
			cache::CatapultCacheDelta m_cacheDelta;

			validators::ValidatorContext m_validatorContext;
			observers::ObserverContext m_observerContext;

			ProcessingNotificationSubscriber m_sub;
		};
	}

	// region basic

	TEST(TEST_CLASS, AggregateResultIsInitiallySuccess) {
		// Act: create a subscriber (in the context)
		TestContext context;

		// Assert:
		EXPECT_EQ(ValidationResult::Success, context.sub().result());
		context.assertValidatorCalls({});
		context.assertObserverCalls({});
	}

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

	// endregion

	// region single notification

	TEST(TEST_CLASS, CanProcessSuccessNotification) {
		// Arrange:
		TestContext context;
		auto notification = test::CreateNotification(Notification_Type_All);

		// Act: process notification
		context.sub().notify(notification);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, context.sub().result());
		context.assertValidatorCalls({ Notification_Type_All });
		context.assertObserverCalls({ Notification_Type_All });
	}

	TEST(TEST_CLASS, CanProcessNeutralNotification) {
		// Arrange:
		TestContext context;
		context.setValidationResult(ValidationResult::Neutral);
		auto notification = test::CreateNotification(Notification_Type_All);

		// Act: process notification
		context.sub().notify(notification);

		// Assert: neutral short-circuits
		EXPECT_EQ(ValidationResult::Neutral, context.sub().result());
		context.assertValidatorCalls({ Notification_Type_All });
		context.assertObserverCalls({});
	}

	TEST(TEST_CLASS, CanProcessFailureNotification) {
		// Arrange:
		TestContext context;
		context.setValidationResult(ValidationResult::Failure);
		auto notification = test::CreateNotification(Notification_Type_All);

		// Act: process notification
		context.sub().notify(notification);

		// Assert: failure short-circuits
		EXPECT_EQ(ValidationResult::Failure, context.sub().result());
		context.assertValidatorCalls({ Notification_Type_All });
		context.assertObserverCalls({});
	}

	// endregion

	// region two notifications

	TEST(TEST_CLASS, CanProcessNotificationAfterSuccessNotification) {
		// Arrange:
		TestContext context;
		auto notification = test::CreateNotification(Notification_Type_All);

		// Act: process two notifications
		context.sub().notify(notification);
		context.sub().notify(notification);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, context.sub().result());
		context.assertValidatorCalls({ Notification_Type_All, Notification_Type_All });
		context.assertObserverCalls({ Notification_Type_All, Notification_Type_All });
	}

	TEST(TEST_CLASS, SkipsNotificationAfterNeutralNotification) {
		// Arrange:
		TestContext context;
		context.setValidationResult(ValidationResult::Neutral);
		auto notification = test::CreateNotification(Notification_Type_All);

		// Act: process two notifications
		context.sub().notify(notification);
		context.sub().notify(notification);

		// Assert: neutral short-circuits
		EXPECT_EQ(ValidationResult::Neutral, context.sub().result());
		context.assertValidatorCalls({ Notification_Type_All });
		context.assertObserverCalls({});
	}

	TEST(TEST_CLASS, SkipsNotificationAfterFailureNotification) {
		// Arrange:
		TestContext context;
		context.setValidationResult(ValidationResult::Failure);
		auto notification = test::CreateNotification(Notification_Type_All);

		// Act: process two notifications
		context.sub().notify(notification);
		context.sub().notify(notification);

		// Assert: failure short-circuits
		EXPECT_EQ(ValidationResult::Failure, context.sub().result());
		context.assertValidatorCalls({ Notification_Type_All });
		context.assertObserverCalls({});
	}

	// endregion

	// region channels

	TEST(TEST_CLASS, NoneChannelNotificationIsPassedToNeitherValidatorNorObserver) {
		// Arrange:
		TestContext context;
		auto notification = test::CreateNotification(Notification_Type_None);

		// Act: process notification
		context.sub().notify(notification);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, context.sub().result());
		context.assertValidatorCalls({});
		context.assertObserverCalls({});
	}

	TEST(TEST_CLASS, ValidatorChannelNotificationIsPassedToValidatorOnly) {
		// Arrange:
		TestContext context;
		auto notification = test::CreateNotification(Notification_Type_Validator);

		// Act: process notification
		context.sub().notify(notification);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, context.sub().result());
		context.assertValidatorCalls({ Notification_Type_Validator });
		context.assertObserverCalls({});
	}

	TEST(TEST_CLASS, ObserverChannelNotificationIsPassedToObserverOnly) {
		// Arrange:
		TestContext context;
		context.setValidationResult(ValidationResult::Failure);
		auto notification = test::CreateNotification(Notification_Type_Observer);

		// Act: process notification
		context.sub().notify(notification);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, context.sub().result());
		context.assertValidatorCalls({});
		context.assertObserverCalls({ Notification_Type_Observer });
	}

	TEST(TEST_CLASS, ValidatorChannelNotificationCanTriggerShortCircuiting) {
		// Arrange:
		TestContext context;
		context.setValidationResult(ValidationResult::Failure);
		auto notification1 = test::CreateNotification(Notification_Type_Validator);
		auto notification2 = test::CreateNotification(Notification_Type_All);

		// Act: process two notifications
		context.sub().notify(notification1);
		context.sub().notify(notification2);

		// Assert: failure short-circuits subsequent validators and observers
		EXPECT_EQ(ValidationResult::Failure, context.sub().result());
		context.assertValidatorCalls({ Notification_Type_Validator });
		context.assertObserverCalls({});
	}

	// endregion

	// region undo

	TEST(TEST_CLASS, CannotUndoWhenUndoIsNotEnabled) {
		// Arrange:
		TestContext context;
		auto notification = test::CreateNotification(Notification_Type_All);

		// - process notification
		context.sub().notify(notification);

		// Act + Assert:
		EXPECT_THROW(context.sub().undo(), catapult_runtime_error);
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
		context.sub().enableUndo();

		// Act: undo notification
		context.sub().undo();

		// Assert:
		EXPECT_EQ(ValidationResult::Success, context.sub().result());
		context.assertValidatorCalls({});
		context.assertObserverCalls({});
	}

	NOTIFY_MODE_TRAITS_BASED_TEST(CanUndoSingleNotification) {
		// Arrange:
		TestContext context(TMode);
		context.sub().enableUndo();
		auto notification = test::CreateNotification(Notification_Type_All);

		// - process notification
		context.sub().notify(notification);

		// Act: undo notification
		context.sub().undo();

		// Assert:
		EXPECT_EQ(ValidationResult::Success, context.sub().result());
		context.assertValidatorCalls({ Notification_Type_All });
		context.assertObserverCalls({ Notification_Type_All, Notification_Type_All }, 1);
	}

	NOTIFY_MODE_TRAITS_BASED_TEST(CanUndoMultipleNotifications) {
		// Arrange:
		TestContext context(TMode);
		context.sub().enableUndo();
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
		EXPECT_EQ(ValidationResult::Success, context.sub().result());
		context.assertValidatorCalls({ Notification_Type_All, Notification_Type_All_2, Notification_Type_All_3 });
		context.assertObserverCalls({
			Notification_Type_All, Notification_Type_All_2, Notification_Type_All_3,
			Notification_Type_All_3, Notification_Type_All_2, Notification_Type_All
		}, 3);
	}

	NOTIFY_MODE_TRAITS_BASED_TEST(OnlyObservableNotificationsCanBeUndone) {
		// Arrange:
		TestContext context(TMode);
		context.sub().enableUndo();
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
		EXPECT_EQ(ValidationResult::Success, context.sub().result());
		context.assertValidatorCalls({ Notification_Type_Validator, Notification_Type_All_2 });
		context.assertObserverCalls({
			Notification_Type_All_2, Notification_Type_Observer,
			Notification_Type_Observer, Notification_Type_All_2
		}, 2);
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
		context.sub().enableUndo();
		auto sender = test::GenerateRandomByteArray<Address>();
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto sourceChangeType = model::SourceChangeNotification::SourceChangeType::Absolute;
		auto notification1 = model::SourceChangeNotification(sourceChangeType, 1, sourceChangeType, 1);
		auto notification2 = test::CreateNotification(Notification_Type_All);
		auto notification3 = model::EntityNotification(model::NetworkIdentifier::Private_Test, model::Entity_Type_Block_Normal, 0, 0, 0);
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
		EXPECT_EQ(ValidationResult::Success, context.sub().result());
		context.assertValidatorCalls({ Notification_Type_All, model::Core_Entity_Notification, model::Core_Transaction_Notification });
		context.assertObserverCalls({
			model::Core_Source_Change_Notification, Notification_Type_All, model::Core_Transaction_Notification,
			model::Core_Transaction_Notification, Notification_Type_All, model::Core_Source_Change_Notification
		}, 3);

		// - check data integrity
		context.assertObserverHashes({
			CalculateNotificationHash(notification1), CalculateNotificationHash(notification2), CalculateNotificationHash(notification4),
			CalculateNotificationHash(notification4), CalculateNotificationHash(notification2), CalculateNotificationHash(notification1)
		});
	}

	TEST(TEST_CLASS, UndoIsIdempotent) {
		// Arrange:
		TestContext context;
		context.sub().enableUndo();
		auto notification = test::CreateNotification(Notification_Type_All);

		// - process notification
		context.sub().notify(notification);

		// Act: undo notification
		for (auto i = 0u; i < 5; ++i)
			context.sub().undo();

		// Assert:
		EXPECT_EQ(ValidationResult::Success, context.sub().result());
		context.assertValidatorCalls({ Notification_Type_All });
		context.assertObserverCalls({ Notification_Type_All, Notification_Type_All }, 1);
	}

	TEST(TEST_CLASS, CanOnlyUndoNotificationsAfterEnablingUndo) {
		// Arrange:
		TestContext context;
		auto notification1 = test::CreateNotification(Notification_Type_All);
		auto notification2 = test::CreateNotification(Notification_Type_All_2);
		auto notification3 = test::CreateNotification(Notification_Type_All_3);

		// - process notifications
		context.sub().notify(notification1);
		context.sub().enableUndo();
		context.sub().notify(notification2);
		context.sub().notify(notification3);

		// Act: undo notification
		context.sub().undo();

		// Assert: the first notification is not undone because it was processed before undo was enabled
		EXPECT_EQ(ValidationResult::Success, context.sub().result());
		context.assertValidatorCalls({ Notification_Type_All, Notification_Type_All_2, Notification_Type_All_3 });
		context.assertObserverCalls({
			Notification_Type_All, Notification_Type_All_2, Notification_Type_All_3,
			Notification_Type_All_3, Notification_Type_All_2
		}, 3);
	}
}}
