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
#include "catapult/validators/NotificationValidator.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace test {

	// region FunctionalValidator

	/// Validates \a notification with \a validator.
	template<typename TNotification>
	validators::ValidationResult ValidateNotification(
			const validators::stateless::NotificationValidatorT<TNotification>& validator,
			const TNotification& notification) {
		return validator.validate(notification);
	}

	/// Validates \a notification with \a validator using \a context.
	template<typename TNotification>
	validators::ValidationResult ValidateNotification(
			const validators::stateful::NotificationValidatorT<TNotification>& validator,
			const TNotification& notification,
			const validators::ValidatorContext& context) {
		return validator.validate(notification, context);
	}

	// endregion

	/// Creates a validator context around a \a height, \a network and \a cache.
	constexpr validators::ValidatorContext CreateValidatorContext(
			Height height,
			const model::NetworkInfo& network,
			const cache::ReadOnlyCatapultCache& cache) {
		return validators::ValidatorContext(height, Timestamp(0), network, cache);
	}

	/// Creates a validator context around a \a height and \a cache.
	constexpr validators::ValidatorContext CreateValidatorContext(Height height, const cache::ReadOnlyCatapultCache& cache) {
		return CreateValidatorContext(height, model::NetworkInfo(), cache);
	}

/// Defines common validator tests for a validator with \a NAME.
#define DEFINE_COMMON_VALIDATOR_TESTS(NAME, ...) \
	TEST(NAME##ValidatorTests, CanCreate##NAME##Validator) { \
		auto pValidator = Create##NAME##Validator(__VA_ARGS__); \
		EXPECT_EQ(#NAME "Validator", pValidator->name()); \
	}
}}
