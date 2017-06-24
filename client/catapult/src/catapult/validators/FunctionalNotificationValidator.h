#pragma once
#include "NotificationValidator.h"
#include <functional>

namespace catapult { namespace validators {

	/// A notification validator implementation that wraps a function.
	template<typename TNotification, typename... TArgs>
	class FunctionalNotificationValidatorT : public NotificationValidatorT<TNotification, TArgs...> {
	private:
		using FunctionType = std::function<ValidationResult (const TNotification&, TArgs&&...)>;

	public:
		/// Creates a functional notification validator around \a func with \a name.
		explicit FunctionalNotificationValidatorT(const std::string& name, const FunctionType& func)
				: m_name(name)
				, m_func(func)
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

		ValidationResult validate(const TNotification& notification, TArgs&&... args) const override {
			return m_func(notification, std::forward<TArgs>(args)...);
		}

	private:
		std::string m_name;
		FunctionType m_func;
	};
}}
