#include "WaitFunctions.h"
#include "tests/test/nodeps/Waits.h"
#include <boost/asio/steady_timer.hpp>
#include <boost/asio.hpp>

namespace catapult { namespace test {

	WaitFunction CreateSyncWaitFunction(uint32_t waitMillis) {
		return [waitMillis](const auto&, const auto& shouldWait) -> void {
			while (shouldWait())
				Sleep(waitMillis);
		};
	}

	namespace {
		void WaitAsync(
				boost::asio::io_service& service,
				const ShouldWaitPredicate& shouldWait,
				uint32_t waitMillis) {
			if (!shouldWait())
				return;

			auto pTimer = std::make_shared<boost::asio::steady_timer>(service);
			pTimer->expires_from_now(std::chrono::milliseconds(waitMillis));
			pTimer->async_wait([&service, shouldWait, waitMillis](const auto&) {
				WaitAsync(service, shouldWait, waitMillis);
			});
		}
	}

	WaitFunction CreateAsyncWaitFunction(uint32_t waitMillis) {
		return [waitMillis](auto& service, const auto& shouldWait) -> void {
			WaitAsync(service, shouldWait, waitMillis);
		};
	}
}}
