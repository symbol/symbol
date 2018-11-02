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

#include "BlockDifficultyCacheStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"

namespace catapult { namespace cache {

	void BlockDifficultyCacheStorage::Save(const ValueType& info, io::OutputStream& output) {
		io::Write(output, info.BlockHeight);
		io::Write(output, info.BlockTimestamp);
		io::Write(output, info.BlockDifficulty);
	}

	state::BlockDifficultyInfo BlockDifficultyCacheStorage::Load(io::InputStream& input) {
		state::BlockDifficultyInfo info;
		io::Read(input, info.BlockHeight);
		io::Read(input, info.BlockTimestamp);
		io::Read(input, info.BlockDifficulty);
		return info;
	}

	void BlockDifficultyCacheStorage::LoadInto(const state::BlockDifficultyInfo& blockDifficultyInfo, DestinationType& cacheDelta) {
		cacheDelta.insert(blockDifficultyInfo);
	}
}}
