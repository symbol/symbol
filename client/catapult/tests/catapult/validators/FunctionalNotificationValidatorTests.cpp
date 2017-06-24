#include "catapult/validators/FunctionalNotificationValidator.h"
#include "tests/test/nodeps/ParamsCapture.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {
	using NotificationType = model::AccountPublicKeyNotification;

	TEST(FunctionalNotificationValidatorTests, HasCorrectName) {
		// Act:
		FunctionalNotificationValidatorT<NotificationType> validator("Foo", [](const auto&) {
			return ValidationResult::Success;
		});

		// Assert:
		EXPECT_EQ("Foo", validator.name());
	}

	namespace {
		struct ValidateParams {
		public:
			ValidateParams(const NotificationType& notification, int context)
					: pNotification(&notification)
					, Context(context)
			{}

		public:
			const NotificationType* pNotification;
			int Context;
		};
	}

	TEST(FunctionalNotificationValidatorTests, ValidateDelegatesToFunction) {
		// Arrange:
		test::ParamsCapture<ValidateParams> capture;
		FunctionalNotificationValidatorT<NotificationType, int> validator("Foo", [&](const auto& notification, auto context) {
			capture.push(notification, context);
			return ValidationResult::Success;
		});

		// Act:
		auto publicKey = test::GenerateRandomData<Key_Size>();
		model::AccountPublicKeyNotification notification(publicKey);
		auto result = validator.validate(notification, 7);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);

		ASSERT_EQ(1u, capture.params().size());
		EXPECT_EQ(&notification, capture.params()[0].pNotification);
		EXPECT_EQ(7, capture.params()[0].Context);
	}
}}
