#include "TransactionUtils.h"
#include "Address.h"
#include "NotificationPublisher.h"
#include "NotificationSubscriber.h"
#include "Transaction.h"

namespace catapult { namespace model {

	namespace {
		class AddressCollector : public NotificationSubscriber {
		public:
			explicit AddressCollector(NetworkIdentifier networkIdentifier) : m_networkIdentifier(networkIdentifier)
			{}

		public:
			void notify(const Notification& notification) override {
				if (Core_Register_Account_Address_Notification == notification.Type)
					m_addresses.insert(static_cast<const AccountAddressNotification&>(notification).Address);
				else if (Core_Register_Account_Public_Key_Notification == notification.Type)
					m_addresses.insert(toAddress(static_cast<const AccountPublicKeyNotification&>(notification).PublicKey));
			}

		public:
			const AddressSet& addresses() const {
				return m_addresses;
			}

		private:
			Address toAddress(const Key& publicKey) const {
				return PublicKeyToAddress(publicKey, m_networkIdentifier);
			}

		private:
			NetworkIdentifier m_networkIdentifier;
			model::AddressSet m_addresses;
		};
	}

	model::AddressSet ExtractAddresses(const Transaction& transaction, const NotificationPublisher& notificationPublisher) {
		WeakEntityInfo weakInfo(transaction);
		AddressCollector sub(NetworkIdentifier(weakInfo.entity().Network()));
		notificationPublisher.publish(weakInfo, sub);
		return sub.addresses();
	}
}}
