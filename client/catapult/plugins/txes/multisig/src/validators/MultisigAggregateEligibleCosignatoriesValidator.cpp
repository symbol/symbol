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

#include "Validators.h"
#include "src/cache/MultisigCache.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::AggregateCosignaturesNotification;

	namespace {
		class AggregateCosignaturesChecker {
		public:
			AggregateCosignaturesChecker(
					const Notification& notification,
					const model::TransactionRegistry& transactionRegistry,
					const cache::MultisigCache::CacheReadOnlyType& multisigCache)
					: m_notification(notification)
					, m_transactionRegistry(transactionRegistry)
					, m_multisigCache(multisigCache) {
				m_cosignatories.emplace(&m_notification.SignerPublicKey, false);
				for (auto i = 0u; i < m_notification.CosignaturesCount; ++i)
					m_cosignatories.emplace(&m_notification.CosignaturesPtr[i].SignerPublicKey, false);
			}

		public:
			bool hasIneligibleCosignatories() {
				// find all eligible cosignatories
				const auto* pTransaction = m_notification.TransactionsPtr;
				for (auto i = 0u; i < m_notification.TransactionsCount; ++i) {
					findEligibleCosignatories(pTransaction->SignerPublicKey);

					const auto& transactionPlugin = m_transactionRegistry.findPlugin(pTransaction->Type)->embeddedPlugin();
					for (const auto& requiredCosignatory : transactionPlugin.additionalRequiredCosignatories(*pTransaction))
						findEligibleCosignatories(requiredCosignatory);

					pTransaction = model::AdvanceNext(pTransaction);
				}

				// check if all cosignatories are in fact eligible
				return std::any_of(m_cosignatories.cbegin(), m_cosignatories.cend(), [](const auto& pair) {
					return !pair.second;
				});
			}

		private:
			void findEligibleCosignatories(const Key& publicKey) {
				// if the account is unknown or not multisig, only the public key itself is eligible
				if (!m_multisigCache.contains(publicKey)) {
					markEligible(publicKey);
					return;
				}

				// if the account is a cosignatory only, treat it as non-multisig
				auto multisigIter = m_multisigCache.find(publicKey);
				const auto& multisigEntry = multisigIter.get();
				if (multisigEntry.cosignatoryPublicKeys().empty()) {
					markEligible(publicKey);
					return;
				}

				// if the account is multisig, only its cosignatories are eligible
				for (const auto& cosignatoryPublicKey : multisigEntry.cosignatoryPublicKeys())
					findEligibleCosignatories(cosignatoryPublicKey);
			}

			void markEligible(const Key& key) {
				auto iter = m_cosignatories.find(&key);
				if (m_cosignatories.cend() != iter)
					iter->second = true;
			}

		private:
			const Notification& m_notification;
			const model::TransactionRegistry& m_transactionRegistry;
			const cache::MultisigCache::CacheReadOnlyType& m_multisigCache;
			utils::ArrayPointerFlagMap<Key> m_cosignatories;
		};
	}

	DECLARE_STATEFUL_VALIDATOR(MultisigAggregateEligibleCosignatories, Notification)(
			const model::TransactionRegistry& transactionRegistry) {
		return MAKE_STATEFUL_VALIDATOR(MultisigAggregateEligibleCosignatories, [&transactionRegistry](
				const Notification& notification,
				const ValidatorContext& context) {
			AggregateCosignaturesChecker checker(notification, transactionRegistry, context.Cache.sub<cache::MultisigCache>());
			return checker.hasIneligibleCosignatories() ? Failure_Aggregate_Ineligible_Cosignatories : ValidationResult::Success;
		});
	}
}}
