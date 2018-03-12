#include "HashLockUtils.h"
#include "plugins/txes/lock/src/model/LockNotifications.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/NotificationSubscriber.h"

namespace catapult { namespace unbondedpruning {

	namespace {
		class DependentTransactionCollector : public model::NotificationSubscriber {
		public:
			void notify(const model::Notification& notification) override {
				if (model::Lock_Hash_Notification == notification.Type)
					m_hashes.insert(static_cast<const model::HashLockNotification&>(notification).Hash);
			}

		public:
			const utils::HashSet& hashes() const {
				return m_hashes;
			}

		private:
			utils::HashSet m_hashes;
		};
	}

	utils::HashSet FindDependentTransactionHashes(
			const model::TransactionInfo& transactionInfo,
			const model::NotificationPublisher& notificationPublisher) {
		model::WeakEntityInfo weakTransactionInfo(*transactionInfo.pEntity, transactionInfo.EntityHash);
		DependentTransactionCollector sub;
		notificationPublisher.publish(weakTransactionInfo, sub);
		return sub.hashes();
	}
}}
