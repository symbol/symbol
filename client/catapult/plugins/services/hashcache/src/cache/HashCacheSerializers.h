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
#include "catapult/cache/CacheSerializerAdapter.h"
#include "catapult/state/TimestampedHash.h"

namespace catapult { namespace cache {

	/// Primary serializer for hash cache.
	struct HashCachePrimarySerializer {
	public:
		/// Serializes to string.
		/// \note Returns empty value because serialized key is actual value.
		static std::string SerializeValue(const typename state::TimestampedHash&) {
			return std::string();
		}

		/// Converts \a timestampedHash to pruning boundary.
		static uint64_t KeyToBoundary(const state::TimestampedHash& timestampedHash) {
			return timestampedHash.Time.unwrap();
		}

		// DeserializeValue is not needed because code using it shouldn't be generated for hash cache
	};
}}
