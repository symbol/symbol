#pragma once
#include "NotificationObserver.h"
#include <vector>

namespace catapult { namespace observers {

	/// A strongly typed aggregate notification observer.
	template<typename TNotification>
	class AggregateNotificationObserverT : public NotificationObserverT<TNotification> {
	public:
		/// Gets the names of all sub observers.
		virtual std::vector<std::string> names() const = 0;
	};
}}
