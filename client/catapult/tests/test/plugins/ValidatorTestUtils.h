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
	constexpr validators::ValidatorContext CreateValidatorContext(
			Height height,
			const cache::ReadOnlyCatapultCache& cache) {
		return CreateValidatorContext(height, model::NetworkInfo(), cache);
	}

/// Defines common validator tests for a validator with \a NAME.
#define DEFINE_COMMON_VALIDATOR_TESTS(NAME, ...) \
	TEST(NAME##ValidatorTests, CanCreate##NAME##Validator) { \
		auto pValidator = Create##NAME##Validator(__VA_ARGS__); \
		EXPECT_EQ(#NAME "Validator", pValidator->name()); \
	}
}}
