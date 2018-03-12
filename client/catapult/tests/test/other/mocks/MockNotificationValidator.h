#pragma once
#include "catapult/validators/ValidatorTypes.h"
#include <vector>

namespace catapult { namespace mocks {

	/// Base of mock notification validators.
	class BasicMockNotificationValidator {
	protected:
		/// Creates a mock validator.
		BasicMockNotificationValidator()
				: m_result(validators::ValidationResult::Success)
				, m_triggerOnSpecificType(false)
		{}

	public:
		/// Returns collected notification types.
		const auto& notificationTypes() const {
			return m_notificationTypes;
		}

	public:
		/// Gets the result for a notification with the specified type (\a notificationType).
		validators::ValidationResult getResultForType(model::NotificationType notificationType) const {
			m_notificationTypes.push_back(notificationType);
			return m_triggerOnSpecificType && m_triggerType != notificationType
					? validators::ValidationResult::Success
					: m_result;
		}

	public:
		/// Sets the result of validate to \a result.
		void setResult(validators::ValidationResult result) {
			m_result = result;
		}

		/// Sets the result of validate for a specific notification type (\a triggerType) to \a result.
		void setResult(validators::ValidationResult result, model::NotificationType triggerType) {
			setResult(result);
			m_triggerOnSpecificType = true;
			m_triggerType = triggerType;
		}

	private:
		validators::ValidationResult m_result;
		bool m_triggerOnSpecificType;
		model::NotificationType m_triggerType;
		mutable std::vector<model::NotificationType> m_notificationTypes;
	};

	/// Mock stateless notification validator that captures information about observed notifications.
	template<typename TNotification>
	class MockStatelessNotificationValidatorT
			: public BasicMockNotificationValidator
			, public validators::stateless::NotificationValidatorT<TNotification> {
	public:
		/// Creates a mock validator with a default name.
		MockStatelessNotificationValidatorT() : MockStatelessNotificationValidatorT("MockStatelessNotificationValidatorT")
		{}

		/// Creates a mock validator with \a name.
		explicit MockStatelessNotificationValidatorT(const std::string& name) : m_name(name)
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

		validators::ValidationResult validate(const TNotification& notification) const override {
			return getResultForType(notification.Type);
		}

	private:
		std::string m_name;
	};

	/// Mock stateful notification validator that captures information about observed notifications and contexts.
	template<typename TNotification>
	class MockNotificationValidatorT
			: public BasicMockNotificationValidator
			, public validators::stateful::NotificationValidatorT<TNotification> {
	public:
		/// Creates a mock validator with a default name.
		MockNotificationValidatorT() : MockNotificationValidatorT("MockNotificationValidatorT")
		{}

		/// Creates a mock validator with \a name.
		explicit MockNotificationValidatorT(const std::string& name) : m_name(name)
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

		validators::ValidationResult validate(
				const TNotification& notification,
				const validators::ValidatorContext& context) const override {
			m_contextPointers.push_back(&context);
			return getResultForType(notification.Type);
		}

	public:
		/// Returns collected context pointers.
		const auto& contextPointers() const {
			return m_contextPointers;
		}

	private:
		std::string m_name;
		mutable std::vector<const validators::ValidatorContext*> m_contextPointers;
	};

	using MockNotificationValidator = MockNotificationValidatorT<model::Notification>;
}}
