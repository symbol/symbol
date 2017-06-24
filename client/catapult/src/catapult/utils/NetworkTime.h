#pragma once
#include "catapult/types.h"
#include <chrono>

namespace catapult { namespace utils {
		// Note: a network timestamp is represented as the number of milliseconds since the Epoch_Time.
		//       a unix timestamp is represented as the number of milliseconds since 1970-01-01 00:00:00 UTC.

		/// Represents the number of milliseconds between 1970-01-01 00:00:00 UTC and 2016-04-01 00:00:00 UTC.
		constexpr auto Epoch_Time = std::chrono::duration<int64_t, std::milli>(1459468800000);

		/// Returns the network time, i.e. the number of milliseconds since Epoch_Time.
		Timestamp NetworkTime();

		/// Given a unix \a timestamp, returns the corresponding network timestamp
		/// that the unix timestamp represents.
		Timestamp ToNetworkTime(const Timestamp& timestamp);

		/// Given a network \a timestamp, returns the corresponding unix timestamp
		/// that the network timestamp represents.
		Timestamp ToUnixTime(const Timestamp& timestamp);
}}
