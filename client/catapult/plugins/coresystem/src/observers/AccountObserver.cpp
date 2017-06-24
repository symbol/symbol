#include "Observers.h"
#include "src/cache/AccountStateCache.h"
#include "catapult/model/EntityType.h"
#include "catapult/model/TransactionPlugin.h"

namespace catapult { namespace observers {

	namespace {
		class DefaultAccountVisitor {
		public:
			explicit DefaultAccountVisitor(const ObserverContext& context) : m_context(context)
			{}

		public:
			void visit(const Address& address) {
				notify(address);
			}

			void visit(const Key& publicKey) {
				notify(publicKey);
			}

		private:
			template<typename AccountId>
			void notify(const AccountId& accountId) {
				auto& accountStateCache = m_context.Cache.sub<cache::AccountStateCache>();
				if (NotifyMode::Commit == m_context.Mode)
					accountStateCache.addAccount(accountId, m_context.Height);
				else
					accountStateCache.queueRemove(accountId, m_context.Height);
			}

		private:
			const ObserverContext& m_context;
		};
	}

	NotificationObserverPointerT<model::AccountAddressNotification> CreateAccountAddressObserver() {
		return std::make_unique<FunctionalNotificationObserverT<model::AccountAddressNotification>>(
				"AccountAddressObserver",
				[](const auto& notification, const auto& context) {
					DefaultAccountVisitor visitor(context);
					visitor.visit(notification.Address);
				});
	}

	NotificationObserverPointerT<model::AccountPublicKeyNotification> CreateAccountPublicKeyObserver() {
		return std::make_unique<FunctionalNotificationObserverT<model::AccountPublicKeyNotification>>(
				"AccountPublicKeyObserver",
				[](const auto& notification, const auto& context) {
					DefaultAccountVisitor visitor(context);
					visitor.visit(notification.PublicKey);
				});
	}
}}
