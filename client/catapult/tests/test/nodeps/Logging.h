#pragma once
#include "catapult/utils/Logging.h"

namespace catapult { namespace test {

	/// An RAII class for managing a global log filter.
	/// \note This is useful for some stress tests that produce a lot of debug logs.
	class GlobalLogFilter {
	public:
		/// Creates a global log filter to suppress all logs less than \a level.
		explicit GlobalLogFilter(utils::LogLevel level);

		/// Destroys the global log filter.
		~GlobalLogFilter();
	};
}}
