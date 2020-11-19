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

#include "catapult/validators/FunctionalNotificationValidator.h"
#include "tests/test/nodeps/ParamsCapture.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS FunctionalNotificationValidatorTests

	using NotificationType = model::AccountPublicKeyNotification;

	TEST(TEST_CLASS, HasCorrectName) {
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

	TEST(TEST_CLASS, ValidateDelegatesToFunction) {
		// Arrange:
		test::ParamsCapture<ValidateParams> capture;
		FunctionalNotificationValidatorT<NotificationType, int> validator("Foo", [&](const auto& notification, auto context) {
			capture.push(notification, context);
			return ValidationResult::Success;
		});

		// Act:
		auto publicKey = test::GenerateRandomByteArray<Key>();
		model::AccountPublicKeyNotification notification(publicKey);
		auto result = validator.validate(notification, 7);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);

		ASSERT_EQ(1u, capture.params().size());
		EXPECT_EQ(&notification, capture.params()[0].pNotification);
		EXPECT_EQ(7, capture.params()[0].Context);
	}
}}
