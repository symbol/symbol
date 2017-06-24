#include "NetworkTime.h"
#include "Casting.h"
#include <limits>

namespace catapult { namespace utils {

	Timestamp NetworkTime() {
		auto now = std::chrono::system_clock::now().time_since_epoch();
		auto nowMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now);
		return Timestamp(static_cast<uint64_t>((nowMillis - Epoch_Time).count()));
	}

	Timestamp ToNetworkTime(const Timestamp& timestamp) {
		if (timestamp.unwrap() < static_cast<uint64_t>(Epoch_Time.count()))
			CATAPULT_THROW_INVALID_ARGUMENT_1("Unix timestamp must be after epoch time", timestamp);

		return Timestamp(static_cast<uint64_t>(timestamp.unwrap() - Epoch_Time.count()));
	}

	Timestamp ToUnixTime(const Timestamp& timestamp) {
		if (std::numeric_limits<uint64_t>::max() - Epoch_Time.count() < timestamp.unwrap())
			CATAPULT_THROW_INVALID_ARGUMENT_1("overflow detected in ToUnixTime", timestamp);

		return Timestamp(static_cast<uint64_t>(Epoch_Time.count() + timestamp.unwrap()));
	}
}}
