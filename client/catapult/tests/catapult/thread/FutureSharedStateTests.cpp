/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "catapult/thread/detail/FutureSharedState.h"
#include "tests/TestHarness.h"
#include <thread>

namespace catapult { namespace thread {

	using namespace detail;

#define TEST_CLASS FutureSharedStateTests

	// region shared_state - basic

	TEST(TEST_CLASS, StateIsInitiallyNotReady) {
		// Arrange:
		shared_state<int> state;

		// Assert:
		EXPECT_FALSE(state.is_ready());
	}

	TEST(TEST_CLASS, CannotSetMultipleContinuations) {
		// Arrange:
		shared_state<int> state;
		state.set_continuation([](const auto&) {});

		// Act + Assert: attempting to set a second continuation throws
		EXPECT_THROW(state.set_continuation([](const auto&) {}), std::logic_error);
	}

	TEST(TEST_CLASS, StatePassedToContinuationHasMovedData) {
		// Arrange:
		auto pInt = std::make_unique<int>(7);
		auto pIntRaw = pInt.get();
		std::unique_ptr<int> pIntRawFromContinuation;

		shared_state<std::unique_ptr<int>> state;
		state.set_value(std::move(pInt));
		state.set_continuation([&pIntRawFromContinuation](const auto& pState) {
			pIntRawFromContinuation = pState->get();
		});

		// Act:
		state.get();

		// Assert: state is ready and continuation was passed original data
		EXPECT_TRUE(state.is_ready());
		EXPECT_EQ(pIntRaw, pIntRawFromContinuation.get());
		EXPECT_EQ(7, *pIntRawFromContinuation);
	}

	// endregion

	// region get / set value scenarios

	namespace {
		struct ValueTraits {
			using ValueType = int;
			using SharedStateType = shared_state<ValueType>;

			static void SetState(SharedStateType& state, int value) {
				state.set_value(std::move(value));
			}

			static void AssertState(SharedStateType& state, int value) {
				// Assert:
				EXPECT_EQ(value, state.get());
			}

			static void AssertMovedState(SharedStateType& state, int value) {
				// Assert: since the type does not deform on move, the original value should persist
				EXPECT_EQ(value, state.get());
			}
		};

		struct MoveOnlyValueTraits {
			using ValueType = std::unique_ptr<int>;
			using SharedStateType = shared_state<ValueType>;

			static void SetState(SharedStateType& state, int value) {
				state.set_value(std::make_unique<int>(value));
			}

			static void AssertState(SharedStateType& state, int value) {
				// Assert: only a single get is allowed
				EXPECT_EQ(value, *state.get());
				EXPECT_FALSE(!!state.get());
			}

			static void AssertMovedState(SharedStateType& state, int) {
				// Assert: since the type is move-only, the state should have been moved into the continuation
				EXPECT_FALSE(!!state.get());
			}
		};

		template<typename TValueType>
		struct ExceptionTraitsT {
			using ValueType = TValueType;
			using SharedStateType = shared_state<ValueType>;

			static void SetState(SharedStateType& state, int) {
				state.set_exception(std::make_exception_ptr(std::runtime_error("state exception")));
			}

			static void AssertState(SharedStateType& state, int) {
				// Act + Assert:
				EXPECT_THROW(state.get(), std::runtime_error);
			}

			static void AssertMovedState(SharedStateType& state, int) {
				// Act + Assert:
				EXPECT_THROW(state.get(), std::runtime_error);
			}
		};

		using ExceptionTraits = ExceptionTraitsT<ValueTraits::ValueType>;
		using MoveOnlyExceptionTraits = ExceptionTraitsT<MoveOnlyValueTraits::ValueType>;
	}

#define SHARED_STATE_TEST(TEST_NAME) TEST(TEST_CLASS, SharedState_##TEST_NAME)

#define SHARED_STATE_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	SHARED_STATE_TEST(TEST_NAME##_Value) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ValueTraits>(); } \
	SHARED_STATE_TEST(TEST_NAME##_Value_MoveOnly) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MoveOnlyValueTraits>(); } \
	SHARED_STATE_TEST(TEST_NAME##_Exception) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExceptionTraits>(); } \
	SHARED_STATE_TEST(TEST_NAME##_Exception_MoveOnly) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MoveOnlyExceptionTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	SHARED_STATE_TRAITS_BASED_TEST(CanGetValueAfterValueIsSet) {
		// Arrange: create a state and set a value on it
		typename TTraits::SharedStateType state;
		TTraits::SetState(state, 7);

		// Assert: the value should be immediately ready
		EXPECT_TRUE(state.is_ready());
		TTraits::AssertState(state, 7);
	}

	SHARED_STATE_TRAITS_BASED_TEST(GetBlocksUntilValueIsSet) {
		test::RunNonDeterministicTest("get blocks until value is set", []() {
			// Arrange:
			typename TTraits::SharedStateType state;

			// - spawn a thread and set a value after some delay
			std::thread([&state] {
				test::Sleep(25);
				TTraits::SetState(state, 8);
			}).detach();

			// Sanity: the state should not be ready
			if (state.is_ready())
				return false;

			// Assert: block until the worker thread sets the state
			TTraits::AssertState(state, 8);
			EXPECT_TRUE(state.is_ready());
			return true;
		});
	}

	SHARED_STATE_TRAITS_BASED_TEST(CannotSetValueAfterStateIsReady) {
		// Arrange: create a state and set a value on it
		typename TTraits::SharedStateType state;
		TTraits::SetState(state, 7);

		// Sanity: the state should be ready
		EXPECT_TRUE(state.is_ready());

		// Act + Assert: setting a value should fail
		EXPECT_THROW(state.set_value(typename TTraits::ValueType()), std::future_error);
	}

	SHARED_STATE_TRAITS_BASED_TEST(CannotSetExceptionAfterStateIsReady) {
		// Arrange: create a state and set a value on it
		typename TTraits::SharedStateType state;
		TTraits::SetState(state, 7);

		// Sanity: the state should be ready
		EXPECT_TRUE(state.is_ready());

		// Act + Assert: setting an exception should fail
		EXPECT_THROW(state.set_exception(std::make_exception_ptr(std::runtime_error("state exception"))), std::future_error);
	}

	SHARED_STATE_TRAITS_BASED_TEST(ContinuationSetBeforeValueIsTriggeredWhenValueIsSet) {
		// Arrange:
		typename TTraits::SharedStateType state;
		bool isContinuationCalled = false;

		// - set a continuation
		state.set_continuation([&isContinuationCalled](const auto& pState) {
			// Assert: the state should be ready immediately
			EXPECT_TRUE(pState->is_ready());
			TTraits::AssertState(*pState, 8);
			isContinuationCalled = true;
		});

		// Sanity: the state is not ready
		EXPECT_FALSE(state.is_ready());

		// Act: set the value after the continuation
		TTraits::SetState(state, 8);

		// Assert: the state is ready and the continuation has been called
		EXPECT_TRUE(state.is_ready());
		EXPECT_TRUE(isContinuationCalled);

		// - the value from the original state was moved into the state passed to the callback
		TTraits::AssertMovedState(state, 8);
	}

	SHARED_STATE_TRAITS_BASED_TEST(ContinuationSetAfterValueIsTriggeredImmediately) {
		// Arrange:
		typename TTraits::SharedStateType state;
		bool isContinuationCalled = false;

		// - set the value
		TTraits::SetState(state, 8);

		// Act: set the continuation
		state.set_continuation([&isContinuationCalled](const auto& pState) {
			// Assert: the state should be ready immediately
			EXPECT_TRUE(pState->is_ready());
			TTraits::AssertState(*pState, 8);
			isContinuationCalled = true;
		});

		// Assert: the state is ready and the continuation has been called
		EXPECT_TRUE(state.is_ready());
		EXPECT_TRUE(isContinuationCalled);

		// - the value from the original state was moved into the state passed to the callback
		TTraits::AssertMovedState(state, 8);
	}

	// endregion
}}
