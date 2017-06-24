#pragma once
#include "catapult/validators/ValidatorTypes.h"
#include <vector>

namespace catapult { namespace mocks {

	/// Mock notification validator that captures information about observed notifications and contexts.
	template<typename TNotification>
	class MockNotificationValidatorT : public validators::stateful::NotificationValidatorT<TNotification> {
	public:
		/// Creates a mock validator with a default name.
		MockNotificationValidatorT() : MockNotificationValidatorT("MockValidatorT")
		{}

		/// Creates a mock validator with \a name.
		explicit MockNotificationValidatorT(const std::string& name)
				: m_name(name)
				, m_result(validators::ValidationResult::Success)
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

		validators::ValidationResult validate(
				const TNotification& notification,
				const validators::ValidatorContext& context) const override {
			m_notificationTypes.push_back(notification.Type);

			m_contextPointers.push_back(&context);
			return m_result;
		}

	public:
		/// Returns collected notification types.
		const auto& notificationTypes() const {
			return m_notificationTypes;
		}

		/// Returns collected context pointers.
		const auto& contextPointers() const {
			return m_contextPointers;
		}

	public:
		/// Sets the result of validate to \a result.
		void setResult(validators::ValidationResult result) {
			m_result = result;
		}

	private:
		std::string m_name;
		validators::ValidationResult m_result;
		mutable std::vector<model::NotificationType> m_notificationTypes;
		mutable std::vector<const validators::ValidatorContext*> m_contextPointers;
	};

	using MockNotificationValidator = MockNotificationValidatorT<model::Notification>;
}}
