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
		auto GetMultisigEntry(cache::MultisigCacheDelta& multisigCache, const Address& address) {
			if (!multisigCache.contains(address))
				multisigCache.insert(state::MultisigEntry(address));

			return multisigCache.find(address);
		}

		class MultisigAccountFacade {
		public:
			MultisigAccountFacade(
					cache::MultisigCacheDelta& multisigCache,
					const Address& multisig,
					const model::ResolverContext& resolvers)
					: m_multisigCache(multisigCache)
					, m_multisig(multisig)
					, m_resolvers(resolvers)
					, m_multisigIter(GetMultisigEntry(m_multisigCache, m_multisig))
					, m_multisigEntry(m_multisigIter.get())
			{}

			~MultisigAccountFacade() {
				removeIfEmpty(m_multisigEntry, m_multisig);
			}

		public:
			void addCosignatory(const UnresolvedAddress& cosignatory) {
				addCosignatory(m_resolvers.resolve(cosignatory));
			}

			void removeCosignatory(const UnresolvedAddress& cosignatory) {
				removeCosignatory(m_resolvers.resolve(cosignatory));
			}

		private:
			void addCosignatory(const Address& cosignatory) {
				auto multisigIter = GetMultisigEntry(m_multisigCache, cosignatory);
				multisigIter.get().multisigAddresses().insert(m_multisig);
				m_multisigEntry.cosignatoryAddresses().insert(cosignatory);
			}

			void removeCosignatory(const Address& cosignatory) {
				m_multisigEntry.cosignatoryAddresses().erase(cosignatory);

				auto multisigIter = m_multisigCache.find(cosignatory);
				auto& cosignatoryEntry = multisigIter.get();
				cosignatoryEntry.multisigAddresses().erase(m_multisig);

				removeIfEmpty(cosignatoryEntry, cosignatory);
			}

			void removeIfEmpty(const state::MultisigEntry& entry, const Address& address) {
				if (entry.cosignatoryAddresses().empty() && entry.multisigAddresses().empty())
					m_multisigCache.remove(address);
			}

		private:
			cache::MultisigCacheDelta& m_multisigCache;
			const Address& m_multisig;
			const model::ResolverContext& m_resolvers;
			cache::MultisigCacheDelta::iterator m_multisigIter;
			state::MultisigEntry& m_multisigEntry;
		};

		void AddAll(MultisigAccountFacade& multisigAccountFacade, const UnresolvedAddress* pAddresses, uint8_t count, bool shouldAdd) {
			for (auto i = 0u; i < count; ++i) {
				const auto& cosignatory = pAddresses[i];
				if (shouldAdd)
					multisigAccountFacade.addCosignatory(cosignatory);
				else
					multisigAccountFacade.removeCosignatory(cosignatory);
			}
		}
	}

	DEFINE_OBSERVER(MultisigCosignatories, Notification, [](const Notification& notification, const ObserverContext& context) {
		auto& multisigCache = context.Cache.sub<cache::MultisigCache>();
		MultisigAccountFacade multisigAccountFacade(multisigCache, notification.Multisig, context.Resolvers);

		auto isCommitMode = NotifyMode::Commit == context.Mode;
		AddAll(multisigAccountFacade, notification.AddressAdditionsPtr, notification.AddressAdditionsCount, isCommitMode);
		AddAll(multisigAccountFacade, notification.AddressDeletionsPtr, notification.AddressDeletionsCount, !isCommitMode);
	});
}}
