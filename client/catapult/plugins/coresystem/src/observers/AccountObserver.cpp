#include "Observers.h"
#include "catapult/cache_core/AccountStateCache.h"
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

	DEFINE_OBSERVER(AccountAddress, model::AccountAddressNotification, [](const auto& notification, const auto& context) {
		DefaultAccountVisitor visitor(context);
		visitor.visit(notification.Address);
	});

	DEFINE_OBSERVER(AccountPublicKey, model::AccountPublicKeyNotification, [](const auto& notification, const auto& context) {
		DefaultAccountVisitor visitor(context);
		visitor.visit(notification.PublicKey);
	});
}}
