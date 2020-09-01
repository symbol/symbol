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

#include "RoundContext.h"
#include "catapult/utils/Hashers.h"

namespace catapult { namespace chain {

	namespace {
		constexpr auto Try_Find_Failure_Result = std::make_pair(model::HeightHashPair(), false);
	}

	bool RoundContext::HeightHashPairComparer::operator()(const model::HeightHashPair& lhs, const model::HeightHashPair& rhs) const {
		return lhs.Height < rhs.Height || (lhs.Height == rhs.Height && lhs.Hash < rhs.Hash);
	}

	size_t RoundContext::HeightHashPairHasher::operator()(const model::HeightHashPair& pair) const {
		return utils::ArrayHasher<Hash256>()(pair.Hash);
	}

	RoundContext::RoundContext(uint64_t weight, uint64_t threshold)
			: m_totalWeight(weight)
			, m_threshold(threshold)
			, m_cumulativePrecommitWeight(0)
	{}

	size_t RoundContext::size() const {
		return m_candidates.size();
	}

	std::pair<model::HeightHashPair, bool> RoundContext::tryFindBestPrevote() const {
		return tryFindLastMatch([threshold = m_threshold](const auto&, const auto& weights) {
			return weights.Prevote >= threshold;
		});
	}

	std::pair<model::HeightHashPair, bool> RoundContext::tryFindBestPrecommit() const {
		return tryFindLastMatch([threshold = m_threshold](const auto&, const auto& weights) {
			return weights.Prevote >= threshold && weights.Precommit >= threshold;
		});
	}

	std::pair<model::HeightHashPair, bool> RoundContext::tryFindEstimate() const {
		auto bestPrevoteResultPair = tryFindBestPrevote();
		if (!bestPrevoteResultPair.second)
			return Try_Find_Failure_Result;

		return tryFindEstimate(bestPrevoteResultPair.first);
	}

	bool RoundContext::isDescendant(const model::HeightHashPair& parentKey, const model::HeightHashPair& childKey) const {
		return m_tree.isDescendant(parentKey, childKey);
	}

	bool RoundContext::isCompletable() const {
		auto bestPrevoteResultPair = tryFindBestPrevote();
		if (!bestPrevoteResultPair.second)
			return false;

		// Erv < g(Vrv) is always completable
		auto estimateResultPair = tryFindEstimate(bestPrevoteResultPair.first);
		if (estimateResultPair.second && bestPrevoteResultPair.first != estimateResultPair.first)
			return true;

		// Erv == g(Vrv) is completable if and only if no child of g(Vrv) can have g(Crv)
		if (canReachPrecommitThreshold(Weights()))
			return false;

		for (auto iter = m_candidates.crbegin(); m_candidates.crend() != iter; ++iter) {
			// check explicitly because isDescendant includes self
			if (bestPrevoteResultPair.first == iter->first)
				continue;

			// if any `best prevote` descendant can reach precommit threshold, round is not yet completable
			if (m_tree.isDescendant(bestPrevoteResultPair.first, iter->first) && canReachPrecommitThreshold(iter->second))
				return false;
		}

		return true;
	}

	RoundContext::Weights RoundContext::weights(const model::HeightHashPair& key) const {
		auto iter = m_candidates.find(key);
		return m_candidates.cend() == iter ? RoundContext::Weights() : iter->second;
	}

	bool RoundContext::canReachPrecommitThreshold(const Weights& weights) const {
		return weights.Precommit + (m_totalWeight - m_cumulativePrecommitWeight) >= m_threshold;
	}

	std::pair<model::HeightHashPair, bool> RoundContext::tryFindLastMatch(const MatchPredicate& predicate) const {
		auto iter = std::find_if(m_candidates.crbegin(), m_candidates.crend(), [predicate](const auto& pair) {
			return predicate(pair.first, pair.second);
		});

		return m_candidates.crend() == iter ? Try_Find_Failure_Result : std::make_pair(iter->first, true);
	}

	std::pair<model::HeightHashPair, bool> RoundContext::tryFindEstimate(const model::HeightHashPair& bestPrevote) const {
		return tryFindLastMatch([this, &bestPrevote](const auto& key, const auto& weights) {
			return m_tree.isDescendant(key, bestPrevote) && canReachPrecommitThreshold(weights);
		});
	}

	void RoundContext::acceptPrevote(Height height, const Hash256* pHashes, size_t count, uint64_t weight) {
		m_tree.addBranch(height, pHashes, count);

		for (auto i = 0u; i < count; ++i) {
			auto key = model::HeightHashPair{ height + Height(i), pHashes[i] };
			auto insertResultPair = m_candidates.emplace(key, Weights());
			insertResultPair.first->second.Prevote += weight;

			// check and update if hash has pending precommit
			auto pendingPrecommitWeightsIter = m_pendingPrecommitWeights.find(key);
			if (m_pendingPrecommitWeights.cend() != pendingPrecommitWeightsIter) {
				auto ancestorKeys = m_tree.findAncestors(key);
				for (const auto& ancestorKey : ancestorKeys)
					m_candidates.find(ancestorKey)->second.Precommit += pendingPrecommitWeightsIter->second;

				m_cumulativePrecommitWeight += pendingPrecommitWeightsIter->second;
				m_pendingPrecommitWeights.erase(pendingPrecommitWeightsIter);
			}
		}
	}

	void RoundContext::acceptPrecommit(Height height, const Hash256& hash, uint64_t weight) {
		auto key = model::HeightHashPair{ height, hash };
		auto ancestorKeys = m_tree.findAncestors(key);
		if (ancestorKeys.empty()) {
			m_pendingPrecommitWeights.emplace(key, 0).first->second += weight;
			return;
		}

		for (const auto& ancestorKey : ancestorKeys)
			m_candidates.find(ancestorKey)->second.Precommit += weight;

		m_cumulativePrecommitWeight += weight;
	}
}}
