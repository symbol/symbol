#pragma once
#include "NotificationObserver.h"
#include <functional>

namespace catapult { namespace observers {

	/// A notification observer implementation that wraps a function.
	template<typename TNotification>
	class FunctionalNotificationObserverT : public NotificationObserverT<TNotification> {
	private:
		using FunctionType = std::function<void (const TNotification&, const ObserverContext&)>;

	public:
		/// Creates a functional notification observer around \a func with \a name.
		explicit FunctionalNotificationObserverT(const std::string& name, const FunctionType& func)
				: m_name(name)
				, m_func(func)
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

		void notify(const TNotification& notification, const ObserverContext& context) const override {
			return m_func(notification, context);
		}

	private:
		std::string m_name;
		FunctionType m_func;
	};
}}
