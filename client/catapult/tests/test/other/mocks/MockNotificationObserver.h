#pragma once
#include "catapult/observers/ObserverTypes.h"
#include "tests/test/core/NotificationTestUtils.h"
#include <vector>

namespace catapult { namespace mocks {

	/// Mock notification observer that captures information about observed notifications and contexts.
	template<typename TNotification>
	class MockNotificationObserverT : public observers::NotificationObserverT<TNotification> {
	public:
		/// Creates a mock observer with a default name.
		MockNotificationObserverT() : MockNotificationObserverT("MockObserverT")
		{}

		/// Creates a mock observer with \a name.
		explicit MockNotificationObserverT(const std::string& name) : m_name(name)
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

		void notify(const TNotification& notification, const observers::ObserverContext& context) const override {
			m_notificationHashes.push_back(test::CalculateNotificationHash(notification));
			m_notificationTypes.push_back(notification.Type);

			if (model::Core_Register_Account_Public_Key_Notification == notification.Type)
				m_accountKeys.push_back(static_cast<const model::AccountPublicKeyNotification&>(notification).PublicKey);

			m_contexts.push_back(context);
			m_contextPointers.push_back(&context);
		}

	public:
		/// Returns collected notification hashes.
		const auto& notificationHashes() const {
			return m_notificationHashes;
		}

		/// Returns collected notification types.
		const auto& notificationTypes() const {
			return m_notificationTypes;
		}

		/// Returns collected account keys.
		const auto& accountKeys() const {
			return m_accountKeys;
		}

		/// Returns collected contexts.
		const auto& contexts() const {
			return m_contexts;
		}

		/// Returns collected context pointers.
		const auto& contextPointers() const {
			return m_contextPointers;
		}

	private:
		std::string m_name;
		mutable std::vector<Hash256> m_notificationHashes;
		mutable std::vector<model::NotificationType> m_notificationTypes;
		mutable std::vector<Key> m_accountKeys;
		mutable std::vector<observers::ObserverContext> m_contexts;
		mutable std::vector<const observers::ObserverContext*> m_contextPointers;
	};

	using MockNotificationObserver = MockNotificationObserverT<model::Notification>;
}}
