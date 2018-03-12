#include "TimestampedHash.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace state {

	std::ostream& operator<<(std::ostream& out, const TimestampedHash& timestampedHash) {
		out << utils::HexFormat(timestampedHash.Hash) << " @ " << timestampedHash.Time;
		return out;
	}
}}
