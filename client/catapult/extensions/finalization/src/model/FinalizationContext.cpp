/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "FinalizationContext.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace model {

	namespace {
		bool Contains(const AddressSet& addresses, const Address& address) {
			return addresses.cend() != addresses.find(address);
		}
	}

	FinalizationContext::FinalizationContext(
			FinalizationEpoch epoch,
			Height height,
			const GenerationHash& generationHash,
			const finalization::FinalizationConfiguration& config,
			const cache::AccountStateCacheView& accountStateCacheView)
			: m_epoch(epoch)
			, m_height(height)
			, m_generationHash(generationHash)
			, m_config(config) {
		const auto& highValueAccounts = accountStateCacheView.highValueAccounts();
		for (const auto& accountHistoryPair : highValueAccounts.accountHistories()) {
			const auto& accountHistory = accountHistoryPair.second;
			auto balance = accountHistory.balance().get(m_height);
			if (Amount() == balance)
				continue;

			auto effectiveVotingPublicKey = FindVotingPublicKeyForEpoch(accountHistory.votingPublicKeys().get(m_height), m_epoch);
			if (VotingKey() == effectiveVotingPublicKey)
				continue;

			const auto& address = accountHistoryPair.first;
			if (config.TreasuryReissuanceEpoch == epoch && Contains(config.TreasuryReissuanceEpochIneligibleVoterAddresses, address)) {
				CATAPULT_LOG(info)
						<< "excluding voting account " << address
						<< " from voting set at epoch " << config.TreasuryReissuanceEpoch;
				continue;
			}

			auto accountView = FinalizationAccountView();
			accountView.Weight = balance;

			m_accounts.emplace(effectiveVotingPublicKey, accountView);
			m_weight = m_weight + balance;
		}
	}

	FinalizationEpoch FinalizationContext::epoch() const {
		return m_epoch;
	}

	Height FinalizationContext::height() const {
		return m_height;
	}

	const GenerationHash& FinalizationContext::generationHash() const {
		return m_generationHash;
	}

	const finalization::FinalizationConfiguration& FinalizationContext::config() const {
		return m_config;
	}

	Amount FinalizationContext::weight() const {
		return m_weight;
	}

	bool FinalizationContext::isEligibleVoter(const VotingKey& votingPublicKey) const {
		return Amount() != lookup(votingPublicKey).Weight;
	}

	FinalizationAccountView FinalizationContext::lookup(const VotingKey& votingPublicKey) const {
		auto iter = m_accounts.find(votingPublicKey);
		return m_accounts.cend() == iter ? FinalizationAccountView() : iter->second;
	}
}}
