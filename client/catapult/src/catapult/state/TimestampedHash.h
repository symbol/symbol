#pragma once
#include "catapult/model/EntityRange.h"
#include "catapult/constants.h"
#include "catapult/types.h"

namespace catapult { namespace state {

#pragma pack(push, 1)

	/// Represents a hash with attached timestamp.
	struct TimestampedHash {
	public:
		/// The hash type.
		using HashType = std::array<uint8_t, Cached_Hash_Size>;

	public:
		/// Creates a timestamped hash.
		constexpr TimestampedHash() : TimestampedHash(Timestamp(0))
		{}

		/// Creates a timestamped hash from a \a timestamp.
		constexpr explicit TimestampedHash(Timestamp timestamp) : Time(timestamp), Hash()
		{}

		/// Creates a timestamped hash from a \a timestamp and a \a hash.
		explicit TimestampedHash(Timestamp timestamp, const Hash256& hash) : Time(timestamp) {
			std::memcpy(Hash.data(), hash.data(), Hash.size());
		}

	public:
		/// The timestamp.
		Timestamp Time;

		/// The hash.
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

	/// Insertion operator for outputting \a timestampedHash to \a out.
	std::ostream& operator<<(std::ostream& out, const TimestampedHash& timestampedHash);

	/// An entity range composed of timestamped hashes.
	using TimestampedHashRange = model::EntityRange<TimestampedHash>;
}}
