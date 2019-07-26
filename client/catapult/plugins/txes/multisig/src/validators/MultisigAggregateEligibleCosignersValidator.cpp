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
		const model::EmbeddedTransaction* AdvanceNext(const model::EmbeddedTransaction* pTransaction) {
			const auto* pTransactionData = reinterpret_cast<const uint8_t*>(pTransaction);
			return reinterpret_cast<const model::EmbeddedTransaction*>(pTransactionData + pTransaction->Size);
		}

		class AggregateCosignaturesChecker {
		public:
			AggregateCosignaturesChecker(
					const Notification& notification,
					const model::TransactionRegistry& transactionRegistry,
					const cache::MultisigCache::CacheReadOnlyType& multisigCache)
					: m_notification(notification)
					, m_transactionRegistry(transactionRegistry)
					, m_multisigCache(multisigCache) {
				m_cosigners.emplace(&m_notification.Signer, false);
				for (auto i = 0u; i < m_notification.CosignaturesCount; ++i)
					m_cosigners.emplace(&m_notification.CosignaturesPtr[i].Signer, false);
			}

		public:
			bool hasIneligibleCosigners() {
				// find all eligible cosigners
				const auto* pTransaction = m_notification.TransactionsPtr;
				for (auto i = 0u; i < m_notification.TransactionsCount; ++i) {
					findEligibleCosigners(pTransaction->Signer);

					const auto& transactionPlugin = m_transactionRegistry.findPlugin(pTransaction->Type)->embeddedPlugin();
					for (const auto& requiredCosigner : transactionPlugin.additionalRequiredCosigners(*pTransaction))
						findEligibleCosigners(requiredCosigner);

					pTransaction = AdvanceNext(pTransaction);
				}

				// check if all cosigners are in fact eligible
				return std::any_of(m_cosigners.cbegin(), m_cosigners.cend(), [](const auto& pair) {
					return !pair.second;
				});
			}

		private:
			void findEligibleCosigners(const Key& publicKey) {
				// if the account is unknown or not multisig, only the public key itself is eligible
				if (!m_multisigCache.contains(publicKey)) {
					markEligible(publicKey);
					return;
				}

				// if the account is a cosignatory only, treat it as non-multisig
				auto multisigIter = m_multisigCache.find(publicKey);
				const auto& multisigEntry = multisigIter.get();
				if (multisigEntry.cosignatories().empty()) {
					markEligible(publicKey);
					return;
				}

				// if the account is multisig, only its cosignatories are eligible
				for (const auto& cosignatoryPublicKey : multisigEntry.cosignatories())
					findEligibleCosigners(cosignatoryPublicKey);
			}

			void markEligible(const Key& key) {
				auto iter = m_cosigners.find(&key);
				if (m_cosigners.cend() != iter)
					iter->second = true;
			}

		private:
			const Notification& m_notification;
			const model::TransactionRegistry& m_transactionRegistry;
			const cache::MultisigCache::CacheReadOnlyType& m_multisigCache;
			utils::ArrayPointerFlagMap<Key> m_cosigners;
		};
	}

	DECLARE_STATEFUL_VALIDATOR(MultisigAggregateEligibleCosigners, Notification)(const model::TransactionRegistry& transactionRegistry) {
		return MAKE_STATEFUL_VALIDATOR(MultisigAggregateEligibleCosigners, [&transactionRegistry](
				const Notification& notification,
				const ValidatorContext& context) {
			AggregateCosignaturesChecker checker(notification, transactionRegistry, context.Cache.sub<cache::MultisigCache>());
			return checker.hasIneligibleCosigners() ? Failure_Aggregate_Ineligible_Cosigners : ValidationResult::Success;
		});
	}
}}
