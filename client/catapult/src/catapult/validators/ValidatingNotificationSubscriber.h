#pragma once
#include "AggregateValidationResult.h"
#include "ValidatorTypes.h"
#include "catapult/model/NotificationSubscriber.h"

namespace catapult { namespace validators {

	/// A notification subscriber that validates notifications.
	class ValidatingNotificationSubscriber : public model::NotificationSubscriber {
	public:
		/// Creates a validating notification subscriber around \a validator.
		explicit ValidatingNotificationSubscriber(const stateless::NotificationValidator& validator)
				: m_validator(validator)
				, m_result(ValidationResult::Success)
		{}

	public:
		/// The aggregate validation result.
		ValidationResult result() const {
			return m_result;
		}

	public:
		void notify(const model::Notification& notification) override {
			if (!IsSet(notification.Type, model::NotificationChannel::Validator))
				return;

			if (IsValidationResultFailure(m_result))
				return;

			auto result = m_validator.validate(notification);
			AggregateValidationResult(m_result, result);
		}

	private:
		const stateless::NotificationValidator& m_validator;
		ValidationResult m_result;
	};
}}
