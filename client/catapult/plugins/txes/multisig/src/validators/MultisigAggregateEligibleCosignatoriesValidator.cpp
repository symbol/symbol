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

	using Notification = model::AggregateCosignaturesNotification;

	namespace {
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
				m_cosignatories.emplace(toAddress(m_notification.SignerPublicKey), false);
				for (auto i = 0u; i < m_notification.CosignaturesCount; ++i)
					m_cosignatories.emplace(toAddress(m_notification.CosignaturesPtr[i].SignerPublicKey), false);
			}

		public:
			bool hasIneligibleCosignatories() {
				// find all eligible cosignatories
				const auto* pTransaction = m_notification.TransactionsPtr;
				for (auto i = 0u; i < m_notification.TransactionsCount; ++i) {
					findEligibleCosignatories(toAddress(pTransaction->SignerPublicKey));

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
			Address toAddress(const Key& publicKey) const {
				return model::PublicKeyToAddress(publicKey, m_context.Network.Identifier);
			}

			void findEligibleCosignatories(const UnresolvedAddress& address) {
				findEligibleCosignatories(m_context.Resolvers.resolve(address));
			}

			void findEligibleCosignatories(const Address& address) {
				// if the account is unknown or not multisig, only the address itself is eligible
				if (!m_multisigCache.contains(address)) {
					markEligible(address);
					return;
				}

				// if the account is a cosignatory only, treat it as non-multisig
				auto multisigIter = m_multisigCache.find(address);
				const auto& multisigEntry = multisigIter.get();
				if (multisigEntry.cosignatoryAddresses().empty()) {
					markEligible(address);
					return;
				}

				// if the account is multisig, only its cosignatories are eligible
				for (const auto& cosignatoryAddress : multisigEntry.cosignatoryAddresses())
					findEligibleCosignatories(cosignatoryAddress);
			}

			void markEligible(const Address& address) {
				auto iter = m_cosignatories.find(address);
				if (m_cosignatories.cend() != iter)
					iter->second = true;
			}

		private:
			const Notification& m_notification;
			const model::TransactionRegistry& m_transactionRegistry;
			const cache::MultisigCache::CacheReadOnlyType& m_multisigCache;
			const ValidatorContext& m_context;
			std::unordered_map<Address, bool, utils::ArrayHasher<Address>> m_cosignatories;
		};
	}

	DECLARE_STATEFUL_VALIDATOR(MultisigAggregateEligibleCosignatories, Notification)(
			const model::TransactionRegistry& transactionRegistry) {
		return MAKE_STATEFUL_VALIDATOR(MultisigAggregateEligibleCosignatories, [&transactionRegistry](
				const Notification& notification,
				const ValidatorContext& context) {
			const auto& multisigCache = context.Cache.sub<cache::MultisigCache>();
			AggregateCosignaturesChecker checker(notification, transactionRegistry, multisigCache, context);
			return checker.hasIneligibleCosignatories() ? Failure_Aggregate_Ineligible_Cosignatories : ValidationResult::Success;
		});
	}
}}
