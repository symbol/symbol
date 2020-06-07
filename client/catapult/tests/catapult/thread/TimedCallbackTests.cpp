/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "catapult/thread/TimedCallback.h"
#include "catapult/thread/IoThreadPool.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace thread {

#define TEST_CLASS TimedCallbackTests

	namespace {
#define TIMED_CALLBACK_RESULT_CODE \
	ENUM_VALUE(Undefined) \
	\
	ENUM_VALUE(Timed_Out) \
	\
	ENUM_VALUE(Completed)

#define ENUM_VALUE(LABEL) LABEL,
		enum class TimedCallbackResultCode {
			TIMED_CALLBACK_RESULT_CODE
		};
#undef ENUM_VALUE

#define ENUM_LIST TIMED_CALLBACK_RESULT_CODE
#define DEFINE_ENUM TimedCallbackResultCode
#include "catapult/utils/MacroBasedEnum.h"
#undef DEFINE_ENUM
#undef ENUM_LIST

		struct TimedCallbackResult {
			std::atomic<TimedCallbackResultCode> Code;
			std::atomic<uint32_t> NumCallbackCalls;
			std::atomic<uint32_t> NumTimeoutHandlerCalls;

			TimedCallbackResult()
					: Code(TimedCallbackResultCode::Undefined)
					, NumCallbackCalls(0)
					, NumTimeoutHandlerCalls(0)
			{}
		};

		class TestContext {
		public:
			using Callback = consumer<TimedCallbackResultCode>;
			using TimedCallback = StrandedTimedCallback<Callback, TimedCallbackResultCode>;

		public:
			explicit TestContext(bool setDefaultTimeoutHandler = true, const std::shared_ptr<int>& pObject = nullptr)
					: m_pPool(test::CreateStartedIoThreadPool()) {
				Callback callback = [&result = m_result, pObject](auto code) {
					result.Code = code;
					++result.NumCallbackCalls;
				};
				m_pTimedCallback = MakeTimedCallback(m_pPool->ioContext(), callback, TimedCallbackResultCode::Timed_Out);

				if (!setDefaultTimeoutHandler)
					return;

				auto timeoutHandler = [&result = m_result, pObject]() { ++result.NumTimeoutHandlerCalls; };
				m_pTimedCallback->setTimeoutHandler(timeoutHandler);
			}

		public:
			TimedCallback& timedCallback() {
				return *m_pTimedCallback;
			}

			void postCallbackWithDelay(uint32_t numMillis) {
				boost::asio::post(m_pPool->ioContext(), [numMillis, pTimedCallback = m_pTimedCallback]() {
					test::Sleep(numMillis);
					pTimedCallback->callback(TimedCallbackResultCode::Completed);
				});
			}

			TimedCallbackResult& wait() {
				test::WaitForUnique(m_pTimedCallback, "m_pTimedCallback");
				return m_result;
			}

		private:
			std::unique_ptr<IoThreadPool> m_pPool;
			std::shared_ptr<TimedCallback> m_pTimedCallback;
			TimedCallbackResult m_result;
		};

		const auto& TriggerTimeoutCallback(TestContext& context, uint32_t numTimeoutMillis, uint32_t numCallbackMillis) {
			context.timedCallback().setTimeout(utils::TimeSpan::FromMilliseconds(numTimeoutMillis));
			context.postCallbackWithDelay(numCallbackMillis);
			return context.wait();
		}

		const auto& TriggerTimeout(TestContext& context) {
			return TriggerTimeoutCallback(context, 5, 50);
		}

		const auto& TriggerCallback(TestContext& context) {
			return TriggerTimeoutCallback(context, 50, 5);
		}

		bool CheckCodes(TimedCallbackResultCode expected, TimedCallbackResultCode actual) {
			if (expected == actual)
				return true;

			CATAPULT_LOG(warning) << "Expected " << expected << " but was " << actual;
			return false;
		}

		void AssertCompletedWithoutTimeout(const TimedCallbackResult& result) {
			EXPECT_EQ(TimedCallbackResultCode::Completed, result.Code);
			EXPECT_EQ(1u, result.NumCallbackCalls);
			EXPECT_EQ(0u, result.NumTimeoutHandlerCalls);
		}

		bool TryAssertCompletedWithoutTimeout(const TimedCallbackResult& result) {
			if (!CheckCodes(TimedCallbackResultCode::Completed, result.Code))
				return false;

			AssertCompletedWithoutTimeout(result);
			return true;
		}

		bool TryAssertTimedOut(const TimedCallbackResult& result) {
			if (!CheckCodes(TimedCallbackResultCode::Timed_Out, result.Code))
				return false;

			EXPECT_EQ(TimedCallbackResultCode::Timed_Out, result.Code);
			EXPECT_EQ(1u, result.NumCallbackCalls);
			EXPECT_EQ(1u, result.NumTimeoutHandlerCalls);
			return true;
		}

		void RunNonDeterministicTriggerTest(const predicate<>& test) {
			// Assert: non-deterministic because timeout trigger is elapsed time
			test::RunNonDeterministicTest("Timeout", test);
		}
	}

	TEST(TEST_CLASS, CanExecuteTimedCallbackWithoutSettingTimeout) {
		// Arrange:
		TestContext context;

		// Act: invoke the callback without setting the timeout
		context.postCallbackWithDelay(5);
		const auto& result = context.wait();

		// Assert: the callback completed and did not time out
		//         (this test is deterministic because a timeout is not set)
		AssertCompletedWithoutTimeout(result);
	}

	TEST(TEST_CLASS, CanExecuteTimedCallbackWhenTimerCompletesBeforeCallback) {
		RunNonDeterministicTriggerTest([&]() {
			// Arrange:
			TestContext context;

			// Act: trigger the timer before the callback
			const auto& result = TriggerTimeout(context);

			// Assert: the callback timed out
			return TryAssertTimedOut(result);
		});
	}

	TEST(TEST_CLASS, CanExecuteTimedCallbackWhenTimerCompletesAfterCallback) {
		RunNonDeterministicTriggerTest([&]() {
			// Arrange:
			TestContext context;

			// Act: trigger the timer after the callback
			const auto& result = TriggerCallback(context);

			// Assert: the callback completed and did not time out
			return TryAssertCompletedWithoutTimeout(result);
		});
	}

	TEST(TEST_CLASS, CanExecuteTimedCallbackWhenTimerCompletesLongAfterCallback) {
		// Arrange:
		TestContext context;

		// Act: trigger the timer long after the callback (this test is checking that the timer is cancelled)
		const auto& result = TriggerTimeoutCallback(context, 100000, 5);

		// Assert: the callback completed and did not time out
		//         (this test is deterministic because the timeout is set very far in the future)
		AssertCompletedWithoutTimeout(result);
	}

	TEST(TEST_CLASS, CanExecuteTimedCallbackWhenTimerCompletesWithCallback) {
		// Arrange:
		TestContext context;

		// Act: trigger the timer at the same time as the callback
		const auto& result = TriggerTimeoutCallback(context, 5, 5);

		// Assert: the callback was only called once
		//         (the exact result is intentionally a race condition)
		EXPECT_EQ(1u, result.NumCallbackCalls);
	}

	TEST(TEST_CLASS, CanTimeoutWhenTimeoutHandlerIsNotSet) {
		RunNonDeterministicTriggerTest([&]() {
			// Arrange: create a context without setting a timeout handler
			TestContext context(false);

			// Act: trigger a timeout
			const auto& result = TriggerTimeout(context);
			if (!CheckCodes(TimedCallbackResultCode::Timed_Out, result.Code))
				return false;

			// Assert: the callback timed out but the handler wasn't called (because it was unset)
			EXPECT_EQ(TimedCallbackResultCode::Timed_Out, result.Code);
			EXPECT_EQ(1u, result.NumCallbackCalls);
			EXPECT_EQ(0u, result.NumTimeoutHandlerCalls);
			return true;
		});
	}

	TEST(TEST_CLASS, TimeoutOnlyTriggersLastTimeoutHandler) {
		RunNonDeterministicTriggerTest([&]() {
			// Arrange: override the default timeout handler
			TestContext context;
			std::atomic<uint32_t> numOverriddenTimeoutHandlerCalls(0);
			context.timedCallback().setTimeoutHandler([&]() { ++numOverriddenTimeoutHandlerCalls; });

			// Act: trigger a timeout
			const auto& result = TriggerTimeout(context);
			if (!CheckCodes(TimedCallbackResultCode::Timed_Out, result.Code))
				return false;

			// Assert: the callback timed out and only the most recent callback was called
			EXPECT_EQ(TimedCallbackResultCode::Timed_Out, result.Code);
			EXPECT_EQ(1u, result.NumCallbackCalls);
			EXPECT_EQ(0u, result.NumTimeoutHandlerCalls);
			EXPECT_EQ(1u, numOverriddenTimeoutHandlerCalls);
			return true;
		});
	}

	TEST(TEST_CLASS, TimeoutHandlerSetAfterTimeoutIsExecutedWhenSet) {
		RunNonDeterministicTriggerTest([&]() {
			// Arrange:
			TestContext context;

			// Act: trigger a timeout
			TriggerTimeout(context);

			// - change the timeout handler
			std::atomic<uint32_t> numOverriddenTimeoutHandlerCalls(0);
			context.timedCallback().setTimeoutHandler([&]() { ++numOverriddenTimeoutHandlerCalls; });
			const auto& result = context.wait();

			// Assert: the callback timed out and both timeout handlers were called
			if (!TryAssertTimedOut(result))
				return false;

			EXPECT_EQ(1u, numOverriddenTimeoutHandlerCalls);
			return true;
		});
	}

	TEST(TEST_CLASS, TimeoutHandlerSetAfterNonTimeoutIsNotExecutedWhenSet) {
		RunNonDeterministicTriggerTest([&]() {
			// Arrange:
			TestContext context;

			// Act: trigger a callback (non-timeout)
			TriggerCallback(context);

			// - change the timeout handler
			std::atomic<uint32_t> numOverriddenTimeoutHandlerCalls(0);
			context.timedCallback().setTimeoutHandler([&]() { ++numOverriddenTimeoutHandlerCalls; });
			const auto& result = context.wait();

			// Assert: the callback did not time out and neither timeout handler was called
			if (!TryAssertCompletedWithoutTimeout(result))
				return false;

			EXPECT_EQ(0u, numOverriddenTimeoutHandlerCalls);
			return true;
		});
	}

	namespace {
		struct TimeoutTraits {
			static constexpr auto Trigger = TriggerTimeout;
			static constexpr auto TryAssertTrigger = TryAssertTimedOut;
		};

		struct NonTimeoutTraits {
			static constexpr auto Trigger = TriggerCallback;
			static constexpr auto TryAssertTrigger = TryAssertCompletedWithoutTimeout;
		};

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Timeout) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TimeoutTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonTimeout) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NonTimeoutTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()
	}

	TRAITS_BASED_TEST(TimeoutHandlerDestroysCallbacksAfterInvocation) {
		RunNonDeterministicTriggerTest([&]() {
			// Arrange: make sure both handlers capture pObject and extend its life
			auto pObject = std::make_shared<int>(7);
			TestContext context(true, pObject);
			EXPECT_LE(3, pObject.use_count()); // sanity

			// Act: trigger the timed callback
			const auto& result = TTraits::Trigger(context);

			// Assert: both callbacks have been destroyed and the only remaining reference is the local pObject
			EXPECT_EQ(1, pObject.use_count());

			// - the timed callback was triggered as expected
			return TTraits::TryAssertTrigger(result);
		});
	}

	namespace {
		template<typename TFunction>
		void ClearFunction(TFunction& func) {
			TFunction empty;
			func.swap(empty);
		}
	}

	TRAITS_BASED_TEST(TimeoutHandlerDestroysTimeoutHandlerCallbackWhenHandlerIsSetAfterInvocation) {
		RunNonDeterministicTriggerTest([&]() {
			// Arrange: make sure both handlers capture pObject and extend its life
			auto pObject = std::make_shared<int>(7);
			TestContext context(true, pObject);
			EXPECT_LE(3, pObject.use_count()); // sanity

			// Act: trigger the timed callback
			TTraits::Trigger(context);

			// - create a timeout handler that captures pObject
			action timeoutHandler = [pObject]() {};
			EXPECT_EQ(2, pObject.use_count()); // sanity

			// - change the timeout handler (with one that captures the object)
			context.timedCallback().setTimeoutHandler(timeoutHandler);
			ClearFunction(timeoutHandler);

			// - wait for the set to complete
			const auto& result = context.wait();

			// Assert: all three callbacks have been destroyed and the only remaining reference is the local pObject
			EXPECT_EQ(1, pObject.use_count());

			// - the timed callback was triggered as expected
			return TTraits::TryAssertTrigger(result);
		});
	}
}}
