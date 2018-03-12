#pragma once
#include "catapult/utils/ExceptionLogging.h"
#include "catapult/utils/Logging.h"

namespace catapult { namespace thread { class IoServiceThreadPool; } }

namespace catapult { namespace tools {

	/// Gets the value associated with \a future and forwards it to \a action on success.
	/// On failure, \c false is returned and \a description is included in a warning log.
	template<typename TAction, typename TFuture>
	bool UnwrapFutureAndSuppressErrors(const char* description, TFuture&& future, TAction action) {
		try {
			action(future.get());
			return true;
		} catch (...) {
			// suppress
			CATAPULT_LOG(warning) << UNHANDLED_EXCEPTION_MESSAGE(description);
			return false;
		}
	}

	/// Creates a started thread pool with \a numThreads threads.
	std::shared_ptr<thread::IoServiceThreadPool> CreateStartedThreadPool(uint32_t numThreads = 1);
}}
