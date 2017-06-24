#include "catapult/utils/ReentrancyCheckReaderNotificationPolicy.h"
#include "tests/TestHarness.h"
#include <boost/thread.hpp>

namespace catapult { namespace utils {

	TEST(ReentrancyCheckReaderNotificationPolicyTests, CannotAcquireReaderMultipleTimesFromSameThreadWithoutRelease) {
		// Arrange:
		ReentrancyCheckReaderNotificationPolicy policy;
		policy.readerAcquired();

		// Act + Assert:
		EXPECT_THROW(policy.readerAcquired(), reader_reentrancy_error);
	}

	TEST(ReentrancyCheckReaderNotificationPolicyTests, CanAcquireReaderFromSameThreadAfterRelease) {
		// Arrange:
		ReentrancyCheckReaderNotificationPolicy policy;
		policy.readerAcquired();

		// Act + Assert: no exception
		policy.readerReleased();
		policy.readerAcquired();
	}

	TEST(ReentrancyCheckReaderNotificationPolicyTests, CanAcquireReaderMultipleTimesFromMultipleThreads) {
		// Arrange:
		constexpr auto Num_Threads = 10u;
		ReentrancyCheckReaderNotificationPolicy policy;
		boost::thread_group threads;

		// Act:
		std::atomic<size_t> counter(0);
		for (auto i = 0u; i < Num_Threads; ++i) {
			threads.create_thread([&policy, &counter] {
				policy.readerAcquired();
				++counter;
			});
		}

		threads.join_all();

		// Assert:
		EXPECT_EQ(Num_Threads, counter);
	}
}}
