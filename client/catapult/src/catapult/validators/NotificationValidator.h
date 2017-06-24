#pragma once
#include "ValidationResult.h"
#include "catapult/model/Notifications.h"
#include <string>

namespace catapult { namespace validators {

	/// A strongly typed notification validator.
	template<typename TNotification, typename... TArgs>
	class NotificationValidatorT {
	public:
		/// The notification type.
		using NotificationType = TNotification;

	public:
		virtual ~NotificationValidatorT() {}

	public:
		/// Gets the validator name.
		virtual const std::string& name() const = 0;

		/// Validates a single \a notification with contextual information \a args.
		virtual ValidationResult validate(const TNotification& notification, TArgs&&... args) const = 0;
	};
}}
