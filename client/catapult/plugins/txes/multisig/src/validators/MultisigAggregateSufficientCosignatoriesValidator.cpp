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

	using Notification = model::AggregateEmbeddedTransactionNotification;

	namespace {
		enum class OperationType { Normal, Removal, Max };

		OperationType GetOperationType(const model::EmbeddedTransaction& transaction) {
			if (model::Entity_Type_Multisig_Account_Modification != transaction.Type)
				return OperationType::Normal;

			const auto& accountModification = static_cast<const model::EmbeddedMultisigAccountModificationTransaction&>(transaction);
			auto hasAdds = 0 != accountModification.PublicKeyAdditionsCount;
			auto hasDeletes = 0 != accountModification.PublicKeyDeletionsCount;

			return hasDeletes
					? hasAdds ? OperationType::Max : OperationType::Removal
					: OperationType::Normal;
		}

		uint32_t GetMinRequiredCosignatories(const state::MultisigEntry& multisigEntry, OperationType operationType) {
			return OperationType::Max == operationType
					? std::max(multisigEntry.minRemoval(), multisigEntry.minApproval())
					: OperationType::Removal == operationType ? multisigEntry.minRemoval() : multisigEntry.minApproval();
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
				m_cosignatories.emplace(&m_notification.SignerPublicKey);
				for (auto i = 0u; i < m_notification.CosignaturesCount; ++i)
					m_cosignatories.emplace(&m_notification.CosignaturesPtr[i].SignerPublicKey);
			}

		public:
			bool hasSufficientCosignatories() {
				const auto& transactionPlugin = m_transactionRegistry.findPlugin(m_notification.Transaction.Type)->embeddedPlugin();
				auto requiredPublicKeys = transactionPlugin.additionalRequiredCosignatories(m_notification.Transaction);
				requiredPublicKeys.insert(m_notification.Transaction.SignerPublicKey);

				auto operationType = GetOperationType(m_notification.Transaction);
				return std::all_of(requiredPublicKeys.cbegin(), requiredPublicKeys.cend(), [this, operationType](const auto& publicKey) {
					return this->isSatisfied(publicKey, operationType);
				});
			}

		private:
			bool isSatisfied(const Key& publicKey, OperationType operationType) {
				// if the account is unknown or not multisig, fallback to default non-multisig verification
				// (where transaction signer is required to be a cosignatory)
				if (!m_multisigCache.contains(publicKey))
					return m_cosignatories.cend() != m_cosignatories.find(&publicKey);

				// if the account is a cosignatory only, treat it as non-multisig
				auto multisigIter = m_multisigCache.find(publicKey);
				const auto& multisigEntry = multisigIter.get();
				if (multisigEntry.cosignatoryPublicKeys().empty())
					return m_cosignatories.cend() != m_cosignatories.find(&publicKey);

				// if the account is multisig, get the entry and check the number of approvers against the minimum number
				auto numApprovers = 0u;
				for (const auto& cosignatoryPublicKey : multisigEntry.cosignatoryPublicKeys())
					numApprovers += isSatisfied(cosignatoryPublicKey, operationType) ? 1 : 0;

				return numApprovers >= GetMinRequiredCosignatories(multisigEntry, operationType);
			}

		private:
			const Notification& m_notification;
			const model::TransactionRegistry& m_transactionRegistry;
			const cache::MultisigCache::CacheReadOnlyType& m_multisigCache;
			utils::KeyPointerSet m_cosignatories;
		};
	}

	DECLARE_STATEFUL_VALIDATOR(MultisigAggregateSufficientCosignatories, Notification)(
			const model::TransactionRegistry& transactionRegistry) {
		return MAKE_STATEFUL_VALIDATOR(MultisigAggregateSufficientCosignatories, [&transactionRegistry](
				const Notification& notification,
				const ValidatorContext& context) {
			AggregateCosignaturesChecker checker(notification, transactionRegistry, context.Cache.sub<cache::MultisigCache>());
			return checker.hasSufficientCosignatories() ? ValidationResult::Success : Failure_Aggregate_Missing_Cosignatures;
		});
	}
}}
