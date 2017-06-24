#include "Observers.h"
#include "src/cache/MultisigCache.h"

namespace catapult { namespace observers {

	using Notification = model::ModifyMultisigCosignersNotification;

	namespace {
		class MultisigAccountFacade {
		public:
			explicit MultisigAccountFacade(cache::MultisigCacheDelta& multisigCache, const Key& multisigAccountKey)
					: m_multisigCache(multisigCache)
					, m_multisigAccountKey(multisigAccountKey)
					, m_multisigEntry(getMultisigEntry(m_multisigAccountKey))
			{}

			~MultisigAccountFacade() {
				removeIfEmpty(m_multisigEntry, m_multisigAccountKey);
			}

		public:
			void addCosignatory(const Key& cosignatoryKey) {
				getMultisigEntry(cosignatoryKey).multisigAccounts().insert(m_multisigAccountKey);
				m_multisigEntry.cosignatories().insert(cosignatoryKey);
			}

			void removeCosignatory(const Key& cosignatoryKey) {
				m_multisigEntry.cosignatories().erase(cosignatoryKey);

				auto& cosignatoryEntry = m_multisigCache.get(cosignatoryKey);
				cosignatoryEntry.multisigAccounts().erase(m_multisigAccountKey);

				removeIfEmpty(cosignatoryEntry, cosignatoryKey);
			}

		private:
			state::MultisigEntry& getMultisigEntry(const Key& key) {
				if (!m_multisigCache.contains(key))
					m_multisigCache.insert(state::MultisigEntry(key));

				return m_multisigCache.get(key);
			}

			void removeIfEmpty(const state::MultisigEntry& entry, const Key& key) {
				if (entry.cosignatories().empty() && entry.multisigAccounts().empty())
					m_multisigCache.remove(key);
			}

		private:
			cache::MultisigCacheDelta& m_multisigCache;
			const Key& m_multisigAccountKey;
			state::MultisigEntry& m_multisigEntry;
		};
	}

	NotificationObserverPointerT<Notification> CreateModifyMultisigCosignersObserver() {
		return std::make_unique<FunctionalNotificationObserverT<Notification>>(
				"ModifyMultisigCosignersObserver",
				[](const auto& notification, const ObserverContext& context) {
					auto& multisigCache = context.Cache.sub<cache::MultisigCache>();

					MultisigAccountFacade multisigAccountFacade(multisigCache, notification.Signer);
					const auto* pModifications = notification.ModificationsPtr;
					for (auto i = 0u; i < notification.ModificationsCount; ++i) {
						auto isNotificationAdd = model::CosignatoryModificationType::Add == pModifications[i].ModificationType;
						auto isNotificationForward = NotifyMode::Commit == context.Mode;

						if (isNotificationAdd == isNotificationForward)
							multisigAccountFacade.addCosignatory(pModifications[i].CosignatoryPublicKey);
						else
							multisigAccountFacade.removeCosignatory(pModifications[i].CosignatoryPublicKey);
					}
				});
	}
}}
