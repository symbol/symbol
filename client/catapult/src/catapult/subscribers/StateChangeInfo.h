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
#include "catapult/cache/CacheChanges.h"
#include "catapult/model/ChainScore.h"
#include "catapult/types.h"

namespace catapult { namespace subscribers {

	/// State change information.
	struct StateChangeInfo {
	public:
		/// Creates state change information around \a cacheChanges, \a scoreDelta and \a height.
		StateChangeInfo(cache::CacheChanges&& cacheChanges, model::ChainScore::Delta scoreDelta, Height height)
				: CacheChanges(std::move(cacheChanges))
				, ScoreDelta(scoreDelta)
				, Height(height)
		{}

	public:
		/// Cache changes.
		const cache::CacheChanges CacheChanges;

		/// Chain score delta.
		const model::ChainScore::Delta ScoreDelta;

		/// New chain height.
		const catapult::Height Height;
	};
}}
