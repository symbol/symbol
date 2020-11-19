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

#pragma once
#include "BlockStatisticCache.h"
#include "catapult/cache/CacheStorageInclude.h"

namespace catapult { namespace cache {

	/// Policy for saving and loading block statistic cache data.
	struct BlockStatisticCacheStorage : public CacheStorageForBasicInsertRemoveCache<BlockStatisticCacheDescriptor> {
		/// Saves \a statistic to \a output.
		static void Save(const ValueType& statistic, io::OutputStream& output);

		/// Loads a single value from \a input.
		static state::BlockStatistic Load(io::InputStream& input);

		/// Purges \a statistic from \a cacheDelta.
		static void Purge(const ValueType& statistic, DestinationType& cacheDelta);
	};
}}
