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

#include "BlockStatisticCacheStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"

namespace catapult { namespace cache {

	void BlockStatisticCacheStorage::Save(const ValueType& statistic, io::OutputStream& output) {
		io::Write(output, statistic.Height);
		io::Write(output, statistic.Timestamp);
		io::Write(output, statistic.Difficulty);
		io::Write(output, statistic.FeeMultiplier);
	}

	state::BlockStatistic BlockStatisticCacheStorage::Load(io::InputStream& input) {
		state::BlockStatistic statistic;
		io::Read(input, statistic.Height);
		io::Read(input, statistic.Timestamp);
		io::Read(input, statistic.Difficulty);
		io::Read(input, statistic.FeeMultiplier);
		return statistic;
	}

	void BlockStatisticCacheStorage::Purge(const ValueType& statistic, DestinationType& cacheDelta) {
		if (!cacheDelta.contains(statistic))
			return;

		// in order to purge `statistic` from the cache, all infos with larger heights must be purged first
		auto maxHeight = statistic.Height;
		while (cacheDelta.contains(state::BlockStatistic(maxHeight + Height(1))))
			maxHeight = maxHeight + Height(1);

		for (auto height = maxHeight; statistic.Height <= height; height = height - Height(1))
			cacheDelta.remove(height);
	}
}}
