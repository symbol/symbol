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

#include "catapult/thread/Future.h"
#include "tests/TestHarness.h"

namespace catapult { namespace thread {

#define TEST_CLASS FutureTests

	// region promise basic

	TEST(TEST_CLASS, CanCreatePromise) {
		// Act:
		promise<int> promise;

		// Assert:
		EXPECT_TRUE(promise.valid());
	}

	TEST(TEST_CLASS, CanMoveConstructPromise) {
		// Arrange:
		promise<int> promise1;

		// Act:
		auto promise2 = std::move(promise1);

		// Assert:
		EXPECT_FALSE(promise1.valid());
		EXPECT_TRUE(promise2.valid());
	}

	TEST(TEST_CLASS, CanMoveAssignPromise) {
		// Arrange:
		promise<int> promise1;
		promise<int> promise2;

		// Act:
		const auto& result = (promise2 = std::move(promise1));

		// Assert:
		EXPECT_EQ(&promise2, &result);
		EXPECT_FALSE(promise1.valid());
		EXPECT_TRUE(promise2.valid());
	}

	TEST(TEST_CLASS, PromiseCanOnlyCreateSingleFuture) {
		// Arrange:
		promise<int> promise;
		auto future = promise.get_future();

		// Act + Assert:
		EXPECT_THROW(promise.get_future(), std::future_error);
		EXPECT_THROW(promise.get_future(), std::future_error);
	}

	// endregion

	// region future basic

	TEST(TEST_CLASS, CanCreateFuture) {
		// Act:
		future<int> future;

		// Assert:
		EXPECT_FALSE(future.valid());
	}

	TEST(TEST_CLASS, CanCreateFutureFromPromise) {
		// Arrange:
		promise<int> promise;

		// Act:
		auto future = promise.get_future();

		// Assert:
		EXPECT_TRUE(future.valid());
		EXPECT_FALSE(future.is_ready());
	}

	TEST(TEST_CLASS, CanMoveConstructFuture) {
		// Arrange:
		promise<int> promise;
		promise.set_value(6);
		auto future1 = promise.get_future();

		// Act:
		auto future2 = std::move(future1);

		// Assert:
		EXPECT_FALSE(future1.valid());
		EXPECT_TRUE(future2.valid());
		EXPECT_TRUE(future2.is_ready());
		EXPECT_EQ(6, future2.get());
	}

	TEST(TEST_CLASS, CanMoveAssignFuture) {
		// Arrange:
		promise<int> promise;
		promise.set_value(6);
		auto future1 = promise.get_future();
		auto future2 = future<int>(nullptr);

		// Act:
		const auto& result = (future2 = std::move(future1));

		// Assert:
		EXPECT_EQ(&future2, &result);
		EXPECT_FALSE(future1.valid());
		EXPECT_TRUE(future2.valid());
		EXPECT_TRUE(future2.is_ready());
		EXPECT_EQ(6, future2.get());
	}

	// endregion

	// region future continuations

	TEST(TEST_CLASS, CannotSetMultipleContinuations) {
		// Arrange:
		promise<int> promise;
		auto future = promise.get_future();
		future.then([](const auto&) { return 7; });

		// Act + Assert: attempting to set a second continuation throws
		EXPECT_THROW(future.then([](const auto&) { return 7; }), std::logic_error);
		EXPECT_THROW(future.then([](const auto&) { return 7; }), std::logic_error);
	}

	TEST(TEST_CLASS, FuturePassedToContinuationHasMovedData) {
		// Arrange:
		auto pInt = std::make_unique<int>(7);
		auto pIntRaw = pInt.get();
		std::unique_ptr<int> pIntRawFromContinuation;

		promise<std::unique_ptr<int>> promise;
		promise.set_value(std::move(pInt));
		auto future = promise.get_future();
		auto finalFuture = future
			.then([&pIntRawFromContinuation](auto&& f) { pIntRawFromContinuation = f.get(); });

		// Act:
		finalFuture.get();

		// Assert: original future is ready but moved
		EXPECT_TRUE(future.is_ready());
		EXPECT_FALSE(!!future.get());

		// - chained future is ready and was passed original data
		EXPECT_TRUE(finalFuture.is_ready());
		EXPECT_EQ(pIntRaw, pIntRawFromContinuation.get());
		EXPECT_EQ(7, *pIntRawFromContinuation);
	}

	TEST(TEST_CLASS, CanAttachMultipleContinuations) {
		// Arrange:
		promise<int> promise;
		promise.set_value(6);
		auto future = promise.get_future();

		auto finalFuture = future
			.then([](auto&& f) { return f.get() * 2; })
			.then([](auto&& f) { return f.get() + 1; })
			.then([](auto&& f) { return f.get() - 3; })
			.then([](auto&& f) { return f.get() * 8; });

		// Act:
		auto value = finalFuture.get();

		// Assert: both futures should be ready and the outer future has the correct value
		EXPECT_TRUE(future.is_ready());
		EXPECT_TRUE(finalFuture.is_ready());
		EXPECT_EQ(((6 * 2) + 1 - 3) * 8, value);
	}

	TEST(TEST_CLASS, ContinuationCanChangeResultType) {
		// Arrange:
		promise<int> promise;
		promise.set_value(6);
		auto future = promise.get_future();

		auto finalFuture = future
			.then([](auto&& f) { return f.get() / 4.; });

		// Act:
		auto value = finalFuture.get();

		// Assert: both futures should be ready and the outer future has the correct value
		EXPECT_TRUE(future.is_ready());
		EXPECT_TRUE(finalFuture.is_ready());
		EXPECT_EQ(6 / 4., value);
	}

	TEST(TEST_CLASS, ContinuationCanHaveVoidReturnType) {
		// Arrange:
		promise<int> promise;
		promise.set_value(6);
		auto future = promise.get_future();

		int continuationValue = 0;
		auto finalFuture = future
			.then([&continuationValue](auto&& f) { continuationValue = f.get(); });

		// Act:
		finalFuture.get();

		// Assert: both futures should be ready and the outer future was called with the correct value
		EXPECT_TRUE(future.is_ready());
		EXPECT_TRUE(finalFuture.is_ready());
		EXPECT_EQ(6, continuationValue);
	}

	TEST(TEST_CLASS, ContinuationCanThrowException) {
		// Arrange:
		promise<int> promise;
		promise.set_value(6);
		auto future = promise.get_future();

		// note: following future always throws, `if` condition is added to avoid
		// VS false-positive warning that `return` statement done inside then() is unreachable
		auto finalFuture = future
			.then([](auto&& f) {
				if (0 != f.get())
					throw std::runtime_error("future exception");

				return std::move(f);
			});

		// Assert: both futures should be ready and the outer future has the correct exception
		EXPECT_TRUE(future.is_ready());
		EXPECT_TRUE(finalFuture.is_ready());
		EXPECT_THROW(finalFuture.get(), std::runtime_error);
	}

	// endregion

	// region get / set value scenarios

	namespace {
		struct ValueTraits {
			using ValueType = int;
			using PromiseType = promise<ValueType>;
			using FutureType = future<ValueType>;

			static void SetPromise(PromiseType& promise, int value) {
				promise.set_value(std::move(value));
			}

			static auto AssertFuture(FutureType& future, int value) {
				EXPECT_EQ(value, future.get());
				return future.get();
			}

			static void AssertMovedFuture(FutureType& future, int value) {
				// since the type does not deform on move, the original value should persist
				EXPECT_EQ(value, future.get());
			}
		};

		struct MoveOnlyValueTraits {
			using ValueType = std::unique_ptr<int>;
			using PromiseType = promise<ValueType>;
			using FutureType = future<ValueType>;

			static void SetPromise(PromiseType& promise, int value) {
				promise.set_value(std::make_unique<int>(value));
			}

			static auto AssertFuture(FutureType& future, int value) {
				// Act:
				auto pValueFromFuture = future.get();

				// Assert: only a single get is allowed
				EXPECT_EQ(value, *pValueFromFuture);
				EXPECT_FALSE(!!future.get());
				return pValueFromFuture;
			}

			static void AssertMovedFuture(FutureType& future, int) {
				// since the type is move-only, the future value should have been moved into the continuation
				EXPECT_FALSE(!!future.get());
			}
		};

		template<typename TValueType>
		struct ExceptionTraitsT {
			using ValueType = TValueType;
			using PromiseType = promise<ValueType>;
			using FutureType = future<ValueType>;

			static void SetPromise(PromiseType& promise, int) {
				promise.set_exception(std::make_exception_ptr(std::runtime_error("future exception")));
			}

			static auto AssertFuture(FutureType& future, int) {
				EXPECT_THROW(future.get(), std::runtime_error);
				return ValueType();
			}

			static void AssertMovedFuture(FutureType& future, int) {
				EXPECT_THROW(future.get(), std::runtime_error);
			}
		};

		using ExceptionTraits = ExceptionTraitsT<ValueTraits::ValueType>;
		using MoveOnlyExceptionTraits = ExceptionTraitsT<MoveOnlyValueTraits::ValueType>;

		int CreateNewValue(int original, int delta) {
			return original + delta;
		}

		std::unique_ptr<int> CreateNewValue(const std::unique_ptr<int>& pOriginal, int delta) {
			return std::make_unique<int>(*pOriginal + delta);
		}
	}

#define PROMISE_FUTURE_TEST(TEST_NAME) TEST(TEST_CLASS, PromiseFuture_##TEST_NAME)

#define PROMISE_FUTURE_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	PROMISE_FUTURE_TEST(TEST_NAME##_Value) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ValueTraits>(); } \
	PROMISE_FUTURE_TEST(TEST_NAME##_Value_MoveOnly) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MoveOnlyValueTraits>(); } \
	PROMISE_FUTURE_TEST(TEST_NAME##_Exception) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExceptionTraits>(); } \
	PROMISE_FUTURE_TEST(TEST_NAME##_Exception_MoveOnly) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MoveOnlyExceptionTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	PROMISE_FUTURE_TRAITS_BASED_TEST(CanGetValueAfterValueIsSet) {
		// Arrange: create a promise and set a value on it
		typename TTraits::PromiseType promise;
		TTraits::SetPromise(promise, 9);
		auto future = promise.get_future();

		// Assert: the future should be ready immediately
		EXPECT_TRUE(future.is_ready());
		TTraits::AssertFuture(future, 9);
	}

	PROMISE_FUTURE_TRAITS_BASED_TEST(GetBlocksUntilValueIsSet) {
		test::RunNonDeterministicTest("get blocks until value is set", []() {
			// Arrange:
			typename TTraits::PromiseType promise;
			auto future = promise.get_future();

			// - spawn a thread and set a value after some delay
			std::thread([&promise] {
				test::Sleep(25);
				TTraits::SetPromise(promise, 8);
			}).detach();

			// Sanity: the future should not be ready
			if (future.is_ready())
				return false;

			// Assert: block until the worker thread sets the promise
			TTraits::AssertFuture(future, 8);
			EXPECT_TRUE(future.is_ready());
			return true;
		});
	}

	PROMISE_FUTURE_TRAITS_BASED_TEST(CannotSetValueAfterFutureIsReady) {
		// Arrange: create a promise and set a value on it
		typename TTraits::PromiseType promise;
		auto future = promise.get_future();
		TTraits::SetPromise(promise, 7);

		// Sanity: the future should be ready
		EXPECT_TRUE(future.is_ready());

		// Act + Assert: setting a value should fail
		EXPECT_THROW(promise.set_value(typename TTraits::ValueType()), std::future_error);
	}

	PROMISE_FUTURE_TRAITS_BASED_TEST(CannotSetExceptionAfterFutureIsReady) {
		// Arrange: create a promise and set a value on it
		typename TTraits::PromiseType promise;
		auto future = promise.get_future();
		TTraits::SetPromise(promise, 7);

		// Sanity: the future should be ready
		EXPECT_TRUE(future.is_ready());

		// Act + Assert: setting an exception should fail
		EXPECT_THROW(promise.set_exception(std::make_exception_ptr(std::runtime_error("future exception"))), std::future_error);
	}

	namespace {
		template<typename TTraits>
		struct VoidContinuationTraits : public TTraits {
			template<typename T>
			static void CreateContinuationResult(const T&, int) {
				// do nothing, the continuation should have a void result
			}

			template<typename TFuture>
			static void AssertFinalFuture(TFuture&, int) {
				// do nothing, the future should have "void" semantics
			}
		};

		template<typename TTraits>
		struct NonVoidContinuationTraits : public TTraits {
			template<typename T>
			static auto CreateContinuationResult(const T& original, int delta) {
				return CreateNewValue(original, delta);
			}

			template<typename TFuture>
			static void AssertFinalFuture(TFuture& future, int expected) {
				TTraits::AssertFuture(future, expected);
			}
		};

		template<typename TTraits>
		void AssertContinuationSetBeforeValueIsTriggeredWhenValueIsSet() {
			// Arrange:
			typename TTraits::PromiseType promise;
			auto future = promise.get_future();
			bool isContinuationCalled = false;

			// - set a continuation
			auto future2 = future.then([&isContinuationCalled](auto&& callbackFuture) {
				// Assert: the future should be ready immediately
				EXPECT_TRUE(callbackFuture.is_ready());
				auto value = TTraits::AssertFuture(callbackFuture, 8);
				isContinuationCalled = true;
				callbackFuture.get(); // allow exceptions to propagate out
				return TTraits::CreateContinuationResult(value, 1);
			});

			// Sanity: the future is not ready
			EXPECT_FALSE(future.is_ready());

			// Act: set the value after the continuation
			TTraits::SetPromise(promise, 8);

			// Assert: the first future is ready and its value was moved into the continuation
			EXPECT_TRUE(future.is_ready());
			TTraits::AssertMovedFuture(future, 8);

			// - the continuation was called and the second future is ready and has the expected value
			EXPECT_TRUE(isContinuationCalled);
			EXPECT_TRUE(future2.is_ready());
			TTraits::AssertFinalFuture(future2, 9);
		}

		template<typename TTraits>
		void AssertContinuationSetAfterValueIsTriggeredImmediately() {
			// Arrange:
			typename TTraits::PromiseType promise;
			auto future = promise.get_future();
			bool isContinuationCalled = false;

			// - set the value
			TTraits::SetPromise(promise, 8);

			// Act: set the continuation
			auto future2 = future.then([&isContinuationCalled](auto&& callbackFuture) {
				// Assert: the future should be ready immediately
				EXPECT_TRUE(callbackFuture.is_ready());
				auto value = TTraits::AssertFuture(callbackFuture, 8);
				isContinuationCalled = true;
				callbackFuture.get(); // allow exceptions to propagate out
				return TTraits::CreateContinuationResult(value, 1);
			});

			// Assert: the first future is ready and its value was moved into the continuation
			EXPECT_TRUE(future.is_ready());
			TTraits::AssertMovedFuture(future, 8);

			// - the continuation was called and the second future is ready and has the expected value
			EXPECT_TRUE(isContinuationCalled);
			EXPECT_TRUE(future2.is_ready());
			TTraits::AssertFinalFuture(future2, 9);
		}
	}

	PROMISE_FUTURE_TRAITS_BASED_TEST(ContinuationSetBeforeValueIsTriggeredWhenValueIsSet) {
		AssertContinuationSetBeforeValueIsTriggeredWhenValueIsSet<NonVoidContinuationTraits<TTraits>>();
	}

	PROMISE_FUTURE_TRAITS_BASED_TEST(VoidContinuationSetBeforeValueIsTriggeredWhenValueIsSet) {
		AssertContinuationSetBeforeValueIsTriggeredWhenValueIsSet<VoidContinuationTraits<TTraits>>();
	}

	PROMISE_FUTURE_TRAITS_BASED_TEST(ContinuationSetAfterValueIsTriggeredImmediately) {
		AssertContinuationSetAfterValueIsTriggeredImmediately<NonVoidContinuationTraits<TTraits>>();
	}

	PROMISE_FUTURE_TRAITS_BASED_TEST(VoidContinuationSetAfterValueIsTriggeredImmediately) {
		AssertContinuationSetAfterValueIsTriggeredImmediately<VoidContinuationTraits<TTraits>>();
	}

	// endregion

	TEST(TEST_CLASS, MakeReadyFutureMakesImmediatelyAvailableFutureAroundValue) {
		// Act:
		auto future = make_ready_future(17);

		// Assert:
		EXPECT_TRUE(future.is_ready());
		EXPECT_EQ(17, future.get());
	}

	TEST(TEST_CLASS, MakeExceptionalFutureMakesImmediatelyAvailableFutureAroundException) {
		// Act:
		auto future = make_exceptional_future<int>(std::runtime_error("future exception"));

		// Assert:
		EXPECT_TRUE(future.is_ready());
		EXPECT_THROW(future.get(), std::runtime_error);
	}
}}
