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
#include "ApiTypes.h"
#include "catapult/model/ChainScore.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/thread/Future.h"
#include "catapult/utils/NonCopyable.h"

namespace catapult { namespace api {

	/// Chain statistics.
	struct ChainStatistics {
		/// Chain height.
		catapult::Height Height;

		/// Finalized chain height.
		catapult::Height FinalizedHeight;

		/// Chain score.
		model::ChainScore Score;
	};

	/// Api for retrieving chain statistics from a node.
	class ChainApi : public utils::NonCopyable {
	public:
		virtual ~ChainApi() = default;

	public:
		/// Gets the chain statistics.
		virtual thread::future<ChainStatistics> chainStatistics() const = 0;

		/// Gets the hashes starting at \a height but no more than \a maxHashes.
		virtual thread::future<model::HashRange> hashesFrom(Height height, uint32_t maxHashes) const = 0;
	};
}}
