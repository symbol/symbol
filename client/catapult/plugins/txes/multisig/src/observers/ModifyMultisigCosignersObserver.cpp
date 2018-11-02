/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "Observers.h"
#include "src/cache/MultisigCache.h"

namespace catapult { namespace observers {

	using Notification = model::ModifyMultisigCosignersNotification;

	namespace {
		auto GetMultisigEntry(cache::MultisigCacheDelta& multisigCache, const Key& key) {
			if (!multisigCache.contains(key))
				multisigCache.insert(state::MultisigEntry(key));

			return multisigCache.find(key);
		}

		class MultisigAccountFacade {
		public:
			explicit MultisigAccountFacade(cache::MultisigCacheDelta& multisigCache, const Key& multisigAccountKey)
					: m_multisigCache(multisigCache)
					, m_multisigAccountKey(multisigAccountKey)
					, m_multisigIter(GetMultisigEntry(m_multisigCache, m_multisigAccountKey))
					, m_multisigEntry(m_multisigIter.get())
			{}

			~MultisigAccountFacade() {
				removeIfEmpty(m_multisigEntry, m_multisigAccountKey);
			}

		public:
			void addCosignatory(const Key& cosignatoryKey) {
				auto multisigIter = GetMultisigEntry(m_multisigCache, cosignatoryKey);
				multisigIter.get().multisigAccounts().insert(m_multisigAccountKey);
				m_multisigEntry.cosignatories().insert(cosignatoryKey);
			}

			void removeCosignatory(const Key& cosignatoryKey) {
				m_multisigEntry.cosignatories().erase(cosignatoryKey);

				auto multisigIter = m_multisigCache.find(cosignatoryKey);
				auto& cosignatoryEntry = multisigIter.get();
				cosignatoryEntry.multisigAccounts().erase(m_multisigAccountKey);

				removeIfEmpty(cosignatoryEntry, cosignatoryKey);
			}

		private:
			void removeIfEmpty(const state::MultisigEntry& entry, const Key& key) {
				if (entry.cosignatories().empty() && entry.multisigAccounts().empty())
					m_multisigCache.remove(key);
			}

		private:
			cache::MultisigCacheDelta& m_multisigCache;
			const Key& m_multisigAccountKey;
			cache::MultisigCacheDelta::iterator m_multisigIter;
			state::MultisigEntry& m_multisigEntry;
		};
	}

	DEFINE_OBSERVER(ModifyMultisigCosigners, Notification, [](const auto& notification, const ObserverContext& context) {
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
}}
