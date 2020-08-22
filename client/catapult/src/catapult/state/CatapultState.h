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
#include "catapult/model/HeightGrouping.h"

namespace catapult { namespace state {

	/// Stateful catapult information.
	struct CatapultState {
	public:
		/// Creates a start state.
		CatapultState() : NumTotalTransactions(0)
		{}

	public:
		/// Height at which importances were last recalculated.
		model::ImportanceHeight LastRecalculationHeight;

		/// Height of last finalized block.
		Height LastFinalizedHeight;

		/// Current dynamic fee multiplier.
		BlockFeeMultiplier DynamicFeeMultiplier;

		/// Total number of confirmed transactions in chain.
		uint64_t NumTotalTransactions;
	};
}}
