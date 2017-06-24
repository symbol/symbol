#pragma once
#include "NotificationValidator.h"
#include <vector>

namespace catapult { namespace validators {

	/// A strongly typed aggregate notification validator.
	template<typename TNotification, typename... TArgs>
	class AggregateNotificationValidatorT : public NotificationValidatorT<TNotification, TArgs...> {
	public:
		/// Gets the names of all sub validators.
		virtual std::vector<std::string> names() const = 0;
	};
}}
