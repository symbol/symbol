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

#pragma once
#include "FinalizationHashTree.h"
#include "catapult/functions.h"
#include <map>
#include <unordered_map>

namespace catapult { namespace chain {

	/// Context for a finalization round.
	class RoundContext {
	public:
		/// Weights associated with a finalization candidate.
		struct Weights {
			/// Total prevote weight.
			uint64_t Prevote = 0;

			/// Total precommit weight.
			uint64_t Precommit = 0;
		};

	public:
		/// Creates a context around the total round \a weight and \a threshold.
		RoundContext(uint64_t weight, uint64_t threshold);

	public:
		/// Gets the number of finalization candidates.
		size_t size() const;

		/// Finds the candidate with the largest height that has at least threshold prevotes, if any.
		std::pair<model::HeightHashPair, bool> tryFindBestPrevote() const;

		/// Finds the candidate with the largest height that has at least threshold prevotes and precommits, if any.
		std::pair<model::HeightHashPair, bool> tryFindBestPrecommit() const;

		/// Finds the estimated finalization candidate, if any.
		std::pair<model::HeightHashPair, bool> tryFindEstimate() const;

		/// Returns \c true if \a childKey descends from \a parentKey, inclusive.
		bool isDescendant(const model::HeightHashPair& parentKey, const model::HeightHashPair& childKey) const;

		/// Returns \c true if the round is completable.
		bool isCompletable() const;

		/// Gets the weights for \a key.
		Weights weights(const model::HeightHashPair& key) const;

	private:
		using MatchPredicate = predicate<const model::HeightHashPair&, const Weights&>;

		bool canReachPrecommitThreshold(const Weights& weights) const;

		std::pair<model::HeightHashPair, bool> tryFindLastMatch(const MatchPredicate& predicate) const;

		std::pair<model::HeightHashPair, bool> tryFindEstimate(const model::HeightHashPair& bestPrevote) const;

	public:
		/// Accepts a prevote for \a count hashes (\a pHashes) starting at \a height with \a weight.
		void acceptPrevote(Height height, const Hash256* pHashes, size_t count, uint64_t weight);

		/// Accepts a precommit for \a hash at \a height with \a weight.
		void acceptPrecommit(Height height, const Hash256& hash, uint64_t weight);

	private:
		struct HeightHashPairComparer {
			bool operator()(const model::HeightHashPair& lhs, const model::HeightHashPair& rhs) const;
		};

		struct HeightHashPairHasher {
			size_t operator()(const model::HeightHashPair& pair) const;
		};

	private:
		const uint64_t m_totalWeight;
		const uint64_t m_threshold;
		uint64_t m_cumulativePrecommitWeight;

		FinalizationHashTree m_tree;
		std::map<model::HeightHashPair, Weights, HeightHashPairComparer> m_candidates;
		std::unordered_map<model::HeightHashPair, uint64_t, HeightHashPairHasher> m_pendingPrecommitWeights;
	};
}}
