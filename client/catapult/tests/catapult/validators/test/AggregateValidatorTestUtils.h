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
#include "MockTaggedBreadcrumbValidator.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/other/mocks/MockNotificationValidator.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	template<typename TBuilder, typename TAddValidator>
	std::vector<const mocks::MockNotificationValidator*> AddSubValidators(TBuilder& builder, TAddValidator addValidator, size_t count) {
		std::vector<const mocks::MockNotificationValidator*> validators;
		for (auto i = 0u; i < count; ++i) {
			auto pMockValidator = std::make_unique<mocks::MockNotificationValidator>(std::to_string(i));
			validators.push_back(pMockValidator.get());
			addValidator(builder, std::move(pMockValidator));
		}

		return validators;
	}

	/// Asserts that the validator created by \a builder delegates to all sub validators and passes notifications correctly.
	/// \a addValidator is used to add sub validators to \a builder.
	template<typename TBuilder, typename TAddValidator>
	void AssertNotificationsAreForwardedToChildValidators(TBuilder&& builder, TAddValidator addValidator) {
		// Arrange:
		cache::CatapultCache cache({});
		auto cacheView = cache.createView();
		auto context = CreateValidatorContext(Height(123), cacheView.toReadOnly());

		// - create an aggregate with five validators
		auto validators = AddSubValidators(builder, addValidator, 5);
		auto pAggregateValidator = builder.build([](auto) { return false; });

		// - create two notifications
		auto notification1 = CreateNotification(static_cast<model::NotificationType>(7));
		auto notification2 = CreateNotification(static_cast<model::NotificationType>(2));

		// Act:
		pAggregateValidator->validate(notification1, context);
		pAggregateValidator->validate(notification2, context);

		// Assert:
		auto i = 0u;
		for (const auto& pValidator : validators) {
			const auto& notificationTypes = pValidator->notificationTypes();
			const auto message = "validator at " + std::to_string(i);

			ASSERT_EQ(2u, notificationTypes.size()) << message;
			EXPECT_EQ(notification1.Type, notificationTypes[0]) << message;
			EXPECT_EQ(notification2.Type, notificationTypes[1]) << message;
			++i;
		}
	}

	/// Asserts that the validator created by \a builder delegates to all sub validators and passes contexts correctly.
	/// \a addValidator is used to add sub validators to \a builder.
	template<typename TBuilder, typename TAddValidator>
	void AssertContextsAreForwardedToChildValidators(TBuilder&& builder, TAddValidator addValidator) {
		// Arrange:
		cache::CatapultCache cache({});
		auto cacheView = cache.createView();
		auto context = CreateValidatorContext(Height(123), cacheView.toReadOnly());

		// - create an aggregate with five validators
		auto validators = AddSubValidators(builder, addValidator, 5);
		auto pAggregateValidator = builder.build([](auto) { return false; });

		// - create two notifications
		auto notification1 = CreateNotification(static_cast<model::NotificationType>(7));
		auto notification2 = CreateNotification(static_cast<model::NotificationType>(2));

		// Act:
		pAggregateValidator->validate(notification1, context);
		pAggregateValidator->validate(notification2, context);

		// Assert:
		auto i = 0u;
		for (const auto& pValidator : validators) {
			const auto& contextPointers = pValidator->contextPointers();
			const auto message = "validator at " + std::to_string(i);

			ASSERT_EQ(2u, contextPointers.size()) << message;
			EXPECT_EQ(&context, contextPointers[0]) << message;
			EXPECT_EQ(&context, contextPointers[1]) << message;
			++i;
		}
	}
}}
