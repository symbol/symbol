#pragma once
#include "catapult/functions.h"

namespace boost { namespace asio { class io_service; } }

namespace catapult { namespace test {

	/// A predicate that returns \c true if a function should continue waiting.
	using ShouldWaitPredicate = predicate<>;

	/// A wait function.
	using WaitFunction = consumer<boost::asio::io_service&, const ShouldWaitPredicate&>;

	/// Creates a synchronous wait function that waits for intervals of \a waitMillis ms.
	WaitFunction CreateSyncWaitFunction(uint32_t waitMillis);

	/// Creates an asynchronous wait function that waits for intervals of \a waitMillis ms.
	WaitFunction CreateAsyncWaitFunction(uint32_t waitMillis);
}}
