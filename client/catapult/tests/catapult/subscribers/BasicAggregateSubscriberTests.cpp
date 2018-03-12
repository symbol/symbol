#include "catapult/subscribers/BasicAggregateSubscriber.h"
#include "tests/catapult/subscribers/test/AggregateSubscriberTestContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS BasicAggregateSubscriberTests

	namespace {
		// int is used as placeholder for a 'subscriber'
		class CapturingAggregateSubscriber : public BasicAggregateSubscriber<int> {
		public:
			using BasicAggregateSubscriber<int>::BasicAggregateSubscriber;

		public:
			std::vector<int*> captureAll() {
				std::vector<int*> subscribers;
				this->forEach([&subscribers](auto& subscriber) { subscribers.push_back(&subscriber); });
				return subscribers;
			}
		};

		using TestContext = test::AggregateSubscriberTestContext<int, CapturingAggregateSubscriber>;

		void RunDelegationTest(size_t numSubscribers) {
			// Arrange:
			TestContext context(numSubscribers);

			// Act:
			auto subscribers = context.aggregate().captureAll();

			// Assert:
			EXPECT_EQ(numSubscribers, subscribers.size());
			EXPECT_EQ(context.subscribers(), subscribers);
		}
	}

	TEST(TEST_CLASS, CanCreateAggregateAroundZeroSubscribers) {
		// Assert:
		RunDelegationTest(0);
	}

	TEST(TEST_CLASS, CanCreateAggregateAroundSingleSubscriber) {
		// Assert:
		RunDelegationTest(1);
	}

	TEST(TEST_CLASS, CanCreateAggregateAroundMultipleSubscribers) {
		// Assert:
		RunDelegationTest(3);
	}
}}
