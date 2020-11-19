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
#include "catapult/model/EntityRange.h"
#include "catapult/constants.h"
#include "catapult/types.h"

namespace catapult { namespace state {

#pragma pack(push, 1)

	/// Represents a hash with attached timestamp.
	struct TimestampedHash {
	public:
		/// Hash type.
		using HashType = std::array<uint8_t, Cached_Hash_Size>;

	public:
		/// Creates a timestamped hash.
		constexpr TimestampedHash() : TimestampedHash(Timestamp(0))
		{}

		/// Creates a timestamped hash from \a timestamp.
		constexpr explicit TimestampedHash(Timestamp timestamp) : Time(timestamp), Hash()
		{}

		/// Creates a timestamped hash from \a timestamp and \a hash.
		TimestampedHash(Timestamp timestamp, const Hash256& hash) : Time(timestamp) {
			std::memcpy(Hash.data(), hash.data(), Hash.size());
		}

	public:
		/// Timestamp.
		Timestamp Time;

		/// Hash.
		/// \note This might be a partial hash.
		HashType Hash;

	public:
		/// Returns \c true if this timestamped hash is less than \a rhs.
		constexpr bool operator<(const TimestampedHash& rhs) const {
			return Time < rhs.Time || (Time == rhs.Time && Hash < rhs.Hash);
		}

		/// Returns \c true if this timestamped hash is equal to \a rhs.
		constexpr bool operator==(const TimestampedHash& rhs) const {
			return Time == rhs.Time && Hash == rhs.Hash;
		}

		/// Returns \c true if this timestamped hash is not equal to \a rhs.
		constexpr bool operator!=(const TimestampedHash& rhs) const {
			return !(*this == rhs);
		}
	};

#pragma pack(pop)

	/// Wraps \a timestampedHash in raw buffer.
	RawBuffer SerializeKey(const TimestampedHash& timestampedHash);

	/// Insertion operator for outputting \a timestampedHash to \a out.
	std::ostream& operator<<(std::ostream& out, const TimestampedHash& timestampedHash);

	/// Entity range composed of timestamped hashes.
	using TimestampedHashRange = model::EntityRange<TimestampedHash>;
}}
