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
#include "finalization/src/FinalizationConfiguration.h"
#include "catapult/utils/Hashers.h"
#include "catapult/types.h"
#include <unordered_map>

namespace catapult { namespace cache { class AccountStateCacheView; } }

namespace catapult { namespace model {

	/// Account data relevant to finalization.
	struct FinalizationAccountView {
		/// Finalization weight.
		Amount Weight;
	};

	/// Contextual information for finalizing blocks.
	class FinalizationContext {
	public:
		/// Creates a finalization context from \a config, \a accountStateCacheView, information about the last finalized block
		/// (\a height and \a generationHash) and the finalization \a point that is currently being finalized.
		FinalizationContext(
				FinalizationPoint point,
				Height height,
				const GenerationHash& generationHash,
				const finalization::FinalizationConfiguration& config,
				const cache::AccountStateCacheView& accountStateCacheView);

	public:
		/// Gets the finalization point.
		FinalizationPoint point() const;

		/// Gets the height of the last finalized block.
		Height height() const;

		/// Gets the generation hash of the last finalized block.
		const GenerationHash& generationHash() const;

		/// Gets the finalization configuration.
		const finalization::FinalizationConfiguration& config() const;

		/// Gets the total weight of all finalization-eligible accounts.
		Amount weight() const;

		/// Gets the finalization account view associated with \a votingPublicKey.
		FinalizationAccountView lookup(const VotingKey& votingPublicKey) const;

	private:
		FinalizationPoint m_point;
		Height m_height;
		GenerationHash m_generationHash;
		finalization::FinalizationConfiguration m_config;
		Amount m_weight;
		std::unordered_map<VotingKey, FinalizationAccountView, utils::ArrayHasher<VotingKey>> m_accounts;
	};
}}
