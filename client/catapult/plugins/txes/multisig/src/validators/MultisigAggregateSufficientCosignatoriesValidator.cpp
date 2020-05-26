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
#include "catapult/model/Address.h"
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
			auto hasAdds = 0 != accountModification.AddressAdditionsCount;
			auto hasDeletes = 0 != accountModification.AddressDeletionsCount;
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
					const cache::MultisigCache::CacheReadOnlyType& multisigCache,
					const ValidatorContext& context)
					: m_notification(notification)
					, m_transactionRegistry(transactionRegistry)
					, m_multisigCache(multisigCache)
					, m_context(context) {
				m_cosignatories.emplace(toAddress(m_notification.SignerPublicKey));
				for (auto i = 0u; i < m_notification.CosignaturesCount; ++i)
					m_cosignatories.emplace(toAddress(m_notification.CosignaturesPtr[i].SignerPublicKey));
			}

		public:
			bool hasSufficientCosignatories() {
				auto requiredAddresses = findRequiredAddresses();

				auto operationType = GetOperationType(m_notification.Transaction);
				return std::all_of(requiredAddresses.cbegin(), requiredAddresses.cend(), [this, operationType](const auto& address) {
					return this->isSatisfied(address, operationType);
				});
			}

		private:
			Address toAddress(const Key& publicKey) const {
				return model::PublicKeyToAddress(publicKey, m_context.Network.Identifier);
			}

			model::AddressSet findRequiredAddresses() const {
				const auto& transactionPlugin = m_transactionRegistry.findPlugin(m_notification.Transaction.Type)->embeddedPlugin();
				auto requiredUnresolvedAddresses = transactionPlugin.additionalRequiredCosignatories(m_notification.Transaction);

				model::AddressSet requiredAddresses;
				requiredAddresses.emplace(toAddress(m_notification.Transaction.SignerPublicKey));
				for (const auto& address : requiredUnresolvedAddresses)
					requiredAddresses.emplace(m_context.Resolvers.resolve(address));

				return requiredAddresses;
			}

			bool isSatisfied(const Address& address, OperationType operationType) {
				// if the account is unknown or not multisig, fallback to default non-multisig verification
				// (where transaction signer is required to be a cosignatory)
				if (!m_multisigCache.contains(address))
					return m_cosignatories.cend() != m_cosignatories.find(address);

				// if the account is a cosignatory only, treat it as non-multisig
				auto multisigIter = m_multisigCache.find(address);
				const auto& multisigEntry = multisigIter.get();
				if (multisigEntry.cosignatoryAddresses().empty())
					return m_cosignatories.cend() != m_cosignatories.find(address);

				// if the account is multisig, get the entry and check the number of approvers against the minimum number
				auto numApprovers = 0u;
				for (const auto& cosignatoryAddress : multisigEntry.cosignatoryAddresses())
					numApprovers += isSatisfied(cosignatoryAddress, operationType) ? 1 : 0;

				return numApprovers >= GetMinRequiredCosignatories(multisigEntry, operationType);
			}

		private:
			const Notification& m_notification;
			const model::TransactionRegistry& m_transactionRegistry;
			const cache::MultisigCache::CacheReadOnlyType& m_multisigCache;
			const ValidatorContext& m_context;
			model::AddressSet m_cosignatories;
		};
	}

	DECLARE_STATEFUL_VALIDATOR(MultisigAggregateSufficientCosignatories, Notification)(
			const model::TransactionRegistry& transactionRegistry) {
		return MAKE_STATEFUL_VALIDATOR(MultisigAggregateSufficientCosignatories, [&transactionRegistry](
				const Notification& notification,
				const ValidatorContext& context) {
			const auto& multisigCache = context.Cache.sub<cache::MultisigCache>();
			AggregateCosignaturesChecker checker(notification, transactionRegistry, multisigCache, context);
			return checker.hasSufficientCosignatories() ? ValidationResult::Success : Failure_Aggregate_Missing_Cosignatures;
		});
	}
}}
