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
#include "src/model/ModifyMultisigAccountTransaction.h"
#include "catapult/utils/ArraySet.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::AggregateEmbeddedTransactionNotification;

	namespace {
		enum class OperationType { Normal, Removal, Max };

		OperationType GetOperationType(const model::EmbeddedTransaction& transaction) {
			if (model::Entity_Type_Modify_Multisig_Account != transaction.Type)
				return OperationType::Normal;

			bool hasAdds = false;
			bool hasDeletes = false;
			const auto& modifyMultisig = static_cast<const model::EmbeddedModifyMultisigAccountTransaction&>(transaction);
			const auto* pModification = modifyMultisig.ModificationsPtr();
			for (auto i = 0u; i < modifyMultisig.ModificationsCount; ++i) {
				switch (pModification->ModificationType) {
				case model::CosignatoryModificationType::Add:
					hasAdds = true;
					break;

				case model::CosignatoryModificationType::Del:
					hasDeletes = true;
					break;
				}

				++pModification;
			}

			return hasDeletes
					? hasAdds ? OperationType::Max : OperationType::Removal
					: OperationType::Normal;
		}

		uint8_t GetMinRequiredCosigners(const state::MultisigEntry& multisigEntry, OperationType operationType) {
			return OperationType::Max == operationType
					? std::max(multisigEntry.minRemoval(), multisigEntry.minApproval())
					: OperationType::Removal == operationType ? multisigEntry.minRemoval() : multisigEntry.minApproval();
		}

		class AggregateCosignaturesChecker {
		public:
			explicit AggregateCosignaturesChecker(
					const Notification& notification,
					const cache::MultisigCache::CacheReadOnlyType& multisigCache)
					: m_notification(notification)
					, m_multisigCache(multisigCache) {
				m_cosigners.emplace(&m_notification.Signer);
				for (auto i = 0u; i < m_notification.CosignaturesCount; ++i)
					m_cosigners.emplace(&m_notification.CosignaturesPtr[i].Signer);
			}

		public:
			bool hasSufficientCosigners() {
				return isSatisfied(m_notification.Transaction.Signer, GetOperationType(m_notification.Transaction));
			}

		private:
			bool isSatisfied(const Key& publicKey, OperationType operationType) {
				// if the account is unknown or not multisig, fallback to default non-multisig verification
				// (where transaction signer is required to be a cosigner)
				if (!m_multisigCache.contains(publicKey))
					return m_cosigners.cend() != m_cosigners.find(&publicKey);

				// if the account is a cosignatory only, treat it as non-multisig
				auto multisigIter = m_multisigCache.find(publicKey);
				const auto& multisigEntry = multisigIter.get();
				if (multisigEntry.cosignatories().empty())
					return m_cosigners.cend() != m_cosigners.find(&publicKey);

				// if the account is multisig, get the entry and check the number of approvers against the minimum number
				auto numApprovers = 0u;
				for (const auto& cosignatoryPublicKey : multisigEntry.cosignatories())
					numApprovers += isSatisfied(cosignatoryPublicKey, operationType) ? 1 : 0;

				return numApprovers >= GetMinRequiredCosigners(multisigEntry, operationType);
			}

		private:
			const Notification& m_notification;
			const cache::MultisigCache::CacheReadOnlyType& m_multisigCache;
			utils::KeyPointerSet m_cosigners;
		};
	}

	DEFINE_STATEFUL_VALIDATOR(MultisigAggregateSufficientCosigners, [](const auto& notification, const ValidatorContext& context) {
		AggregateCosignaturesChecker checker(notification, context.Cache.sub<cache::MultisigCache>());
		return checker.hasSufficientCosigners() ? ValidationResult::Success : Failure_Aggregate_Missing_Cosigners;
	});
}}
