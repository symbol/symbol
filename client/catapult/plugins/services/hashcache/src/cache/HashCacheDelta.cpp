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

#include "HashCacheDelta.h"

namespace catapult { namespace cache {

	BasicHashCacheDelta::BasicHashCacheDelta(const HashCacheTypes::BaseSetDeltaPointers& hashSets, const HashCacheTypes::Options& options)
			: HashCacheDeltaMixins::Size(*hashSets.pPrimary)
			, HashCacheDeltaMixins::Contains(*hashSets.pPrimary)
			, HashCacheDeltaMixins::BasicInsertRemove(*hashSets.pPrimary)
			, HashCacheDeltaMixins::DeltaElements(*hashSets.pPrimary)
			, m_pOrderedDelta(hashSets.pPrimary)
			, m_retentionTime(options.RetentionTime)
	{}

	utils::TimeSpan BasicHashCacheDelta::retentionTime() const {
		return m_retentionTime;
	}

	deltaset::PruningBoundary<BasicHashCacheDelta::ValueType> BasicHashCacheDelta::pruningBoundary() const {
		return m_pruningBoundary;
	}

	void BasicHashCacheDelta::prune(Timestamp timestamp) {
		auto pruneTime = SubtractNonNegative(timestamp, m_retentionTime);
		m_pruningBoundary = ValueType(pruneTime);
	}
}}
