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

#include "FinalizationContext.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace model {

	namespace {
		VotingKey Find(const std::vector<PinnedVotingKey>& pinnedPublicKeys, FinalizationPoint point) {
			auto iter = std::find_if(pinnedPublicKeys.cbegin(), pinnedPublicKeys.cend(), [point](const auto& pinnedPublicKey) {
				return pinnedPublicKey.StartPoint <= point && point <= pinnedPublicKey.EndPoint;
			});
			return pinnedPublicKeys.cend() != iter ? iter->VotingKey : VotingKey();
		}
	}

	FinalizationContext::FinalizationContext(
			FinalizationPoint point,
			Height height,
			const GenerationHash& generationHash,
			const finalization::FinalizationConfiguration& config,
			const cache::AccountStateCacheView& accountStateCacheView)
			: m_point(point)
			, m_height(height)
			, m_generationHash(generationHash)
			, m_config(config) {
		const auto& highValueAccounts = accountStateCacheView.highValueAccounts();
		for (const auto& accountHistoryPair : highValueAccounts.accountHistories()) {
			const auto& accountHistory = accountHistoryPair.second;
			auto balance = accountHistory.balance().get(m_height);
			if (Amount() == balance)
				continue;

			auto accountView = FinalizationAccountView();
			accountView.Weight = balance;

			m_accounts.emplace(Find(accountHistory.votingPublicKeys().get(m_height), m_point), accountView);
			m_weight = m_weight + balance;
		}
	}

	FinalizationPoint FinalizationContext::point() const {
		return m_point;
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

	FinalizationAccountView FinalizationContext::lookup(const VotingKey& votingPublicKey) const {
		auto iter = m_accounts.find(votingPublicKey);
		return m_accounts.cend() == iter ? FinalizationAccountView() : iter->second;
	}
}}
