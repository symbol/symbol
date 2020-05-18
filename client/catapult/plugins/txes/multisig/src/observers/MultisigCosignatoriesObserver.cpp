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

	using Notification = model::MultisigCosignatoriesNotification;

	namespace {
		auto GetMultisigEntry(cache::MultisigCacheDelta& multisigCache, const Key& key) {
			if (!multisigCache.contains(key))
				multisigCache.insert(state::MultisigEntry(key));

			return multisigCache.find(key);
		}

		class MultisigAccountFacade {
		public:
			MultisigAccountFacade(cache::MultisigCacheDelta& multisigCache, const Key& multisigAccountKey)
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
				multisigIter.get().multisigPublicKeys().insert(m_multisigAccountKey);
				m_multisigEntry.cosignatoryPublicKeys().insert(cosignatoryKey);
			}

			void removeCosignatory(const Key& cosignatoryKey) {
				m_multisigEntry.cosignatoryPublicKeys().erase(cosignatoryKey);

				auto multisigIter = m_multisigCache.find(cosignatoryKey);
				auto& cosignatoryEntry = multisigIter.get();
				cosignatoryEntry.multisigPublicKeys().erase(m_multisigAccountKey);

				removeIfEmpty(cosignatoryEntry, cosignatoryKey);
			}

		private:
			void removeIfEmpty(const state::MultisigEntry& entry, const Key& key) {
				if (entry.cosignatoryPublicKeys().empty() && entry.multisigPublicKeys().empty())
					m_multisigCache.remove(key);
			}

		private:
			cache::MultisigCacheDelta& m_multisigCache;
			const Key& m_multisigAccountKey;
			cache::MultisigCacheDelta::iterator m_multisigIter;
			state::MultisigEntry& m_multisigEntry;
		};

		void AddAll(MultisigAccountFacade& multisigAccountFacade, const Key* pKeys, uint8_t numKeys, bool shouldAdd) {
			for (auto i = 0u; i < numKeys; ++i) {
				const auto& cosignatoryPublicKey = pKeys[i];
				if (shouldAdd)
					multisigAccountFacade.addCosignatory(cosignatoryPublicKey);
				else
					multisigAccountFacade.removeCosignatory(cosignatoryPublicKey);
			}
		}
	}

	DEFINE_OBSERVER(MultisigCosignatories, Notification, [](const Notification& notification, const ObserverContext& context) {
		auto& multisigCache = context.Cache.sub<cache::MultisigCache>();
		MultisigAccountFacade multisigAccountFacade(multisigCache, notification.SignerPublicKey);

		auto isCommitMode = NotifyMode::Commit == context.Mode;
		AddAll(multisigAccountFacade, notification.PublicKeyAdditionsPtr, notification.PublicKeyAdditionsCount, isCommitMode);
		AddAll(multisigAccountFacade, notification.PublicKeyDeletionsPtr, notification.PublicKeyDeletionsCount, !isCommitMode);
	});
}}
