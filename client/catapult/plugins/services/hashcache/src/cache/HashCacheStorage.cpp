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

#include "HashCacheStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"

namespace catapult { namespace cache {

	void HashCacheStorage::Save(const StorageType& element, io::OutputStream& output) {
		io::Write(output, element.Time);
		io::Write(output, element.Hash);
	}

	state::TimestampedHash HashCacheStorage::Load(io::InputStream& input) {
		state::TimestampedHash timestampedHash;
		io::Read(input, timestampedHash.Time);
		io::Read(input, timestampedHash.Hash);
		return timestampedHash;
	}

	void HashCacheStorage::LoadInto(io::InputStream& input, DestinationType& cacheDelta) {
		cacheDelta.insert(Load(input));
	}
}}
