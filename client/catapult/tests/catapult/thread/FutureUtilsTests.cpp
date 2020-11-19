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

#include "catapult/thread/FutureUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace thread {

#define TEST_CLASS FutureUtilsTests

	namespace {
		using IntFuturesVector = std::vector<future<int>>;

		void LogAndSleep(long numMillis) {
			CATAPULT_LOG(debug) << "sleeping for " << numMillis << "ms";
			test::Sleep(numMillis);
		}

		future<int> CreateSleepValueFuture(long numMillis, int value) {
			promise<int> promise;
			auto future = promise.get_future();
			std::thread([promise = std::move(promise), numMillis, value]() mutable {
				LogAndSleep(numMillis);
				CATAPULT_LOG(debug) << "returning " << value << " after sleep";

				promise.set_value(std::move(value));
			}).detach();
			return future;
		}

		future<int> CreateSleepExceptionFuture(long numMillis) {
			promise<int> promise;
			auto future = promise.get_future();
			std::thread([promise = std::move(promise), numMillis]() mutable {
				LogAndSleep(numMillis);
				CATAPULT_LOG(debug) << "throwing exception after sleep";

				auto pException = std::make_exception_ptr(catapult_runtime_error("throwing exception after sleep"));
				promise.set_exception(pException);
			}).detach();
			return future;
		}

		future<std::string> CreateSleepContinuationFuture(long numMillis, future<int>&& valueFuture) {
			auto value = valueFuture.get();

			promise<std::string> promise;
			auto future = promise.get_future();
			std::thread([promise = std::move(promise), numMillis, value]() mutable {
				LogAndSleep(numMillis);
				CATAPULT_LOG(debug) << "returning " << value << " after sleep (continuation)";

				promise.set_value(std::to_string(2 * value));
			}).detach();
			return future;
		}

		void AssertAllAreReady(const std::vector<future<int>>& futures) {
			size_t i = 0;
			for (const auto& future : futures)
				EXPECT_TRUE(future.is_ready()) << "future at index " << i++;
		}
	}

	// region when_all

	namespace {
		struct PairTraits {
			template<typename T>
			static auto WhenAll(future<T>&& future1, future<T>&& future2) {
				return when_all(std::move(future1), std::move(future2));
			}
		};

		struct VectorTraits {
			template<typename T>
			static auto WhenAll(future<T>&& future1, future<T>&& future2) {
				std::vector<future<T>> futures;
				futures.push_back(std::move(future1));
				futures.push_back(std::move(future2));
				return when_all(std::move(futures));
			}
		};
	}

#define WHEN_ALL_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Pair) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PairTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Vector) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VectorTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	WHEN_ALL_TEST(WhenAllDoesNotBlock) {
		// Arrange:
		auto future1 = CreateSleepValueFuture(100, 7);
		auto future2 = CreateSleepValueFuture(50, 11);

		// Act:
		auto aggregateFuture = TTraits::WhenAll(std::move(future1), std::move(future2));

		// Assert:
		EXPECT_FALSE(aggregateFuture.is_ready());
	}

	WHEN_ALL_TEST(WhenAllPropagatesExceptionFromFirstFuture) {
		// Arrange:
		auto future1 = CreateSleepExceptionFuture(100);
		auto future2 = CreateSleepValueFuture(50, 7);

		// Act:
		auto aggregateFuture = TTraits::WhenAll(std::move(future1), std::move(future2));
		auto resultFutures = aggregateFuture.get();

		// Assert:
		EXPECT_TRUE(aggregateFuture.is_ready());
		AssertAllAreReady(resultFutures);

		ASSERT_EQ(2u, resultFutures.size());
		EXPECT_THROW(resultFutures[0].get(), catapult_runtime_error);
		EXPECT_EQ(7, resultFutures[1].get());
	}

	WHEN_ALL_TEST(WhenAllPropagatesExceptionFromLastFuture) {
		// Arrange:
		auto future1 = CreateSleepValueFuture(50, 7);
		auto future2 = CreateSleepExceptionFuture(100);

		// Act:
		auto aggregateFuture = TTraits::WhenAll(std::move(future1), std::move(future2));
		auto resultFutures = aggregateFuture.get();

		// Assert:
		EXPECT_TRUE(aggregateFuture.is_ready());
		AssertAllAreReady(resultFutures);

		ASSERT_EQ(2u, resultFutures.size());
		EXPECT_EQ(7, resultFutures[0].get());
		EXPECT_THROW(resultFutures[1].get(), catapult_runtime_error);
	}

	WHEN_ALL_TEST(WhenAllCollectsResultsFromAllFutures) {
		// Arrange:
		auto future1 = CreateSleepValueFuture(100, 7);
		auto future2 = CreateSleepValueFuture(50, 11);

		// Act:
		auto aggregateFuture = TTraits::WhenAll(std::move(future1), std::move(future2));
		auto resultFutures = aggregateFuture.get();

		// Assert:
		EXPECT_TRUE(aggregateFuture.is_ready());
		AssertAllAreReady(resultFutures);

		ASSERT_EQ(2u, resultFutures.size());
		EXPECT_EQ(7, resultFutures[0].get());
		EXPECT_EQ(11, resultFutures[1].get());
	}

	TEST(TEST_CLASS, WhenAllDoesNotSupportsZeroFutures) {
		// Arrange:
		IntFuturesVector futures;

		// Act + Assert:
		EXPECT_THROW(when_all(std::move(futures)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, WhenAllSupportsSingleFuture) {
		// Arrange:
		IntFuturesVector futures;
		futures.push_back(CreateSleepValueFuture(50, 11));

		// Act:
		auto aggregateFuture = when_all(std::move(futures));
		auto resultFutures = aggregateFuture.get();

		// Assert:
		EXPECT_TRUE(aggregateFuture.is_ready());
		AssertAllAreReady(resultFutures);

		ASSERT_EQ(1u, resultFutures.size());
		EXPECT_EQ(11, resultFutures[0].get());
	}

	TEST(TEST_CLASS, WhenAllSupportsManyFutures) {
		// Arrange: create 10 futures
		constexpr auto Num_Futures = 10u;
		IntFuturesVector futures;
		for (auto i = 0u; i < Num_Futures; ++i) {
			auto sleepMillis = 10 * (i + 1) * (i % 2 ? 2 : 1); // penalize even future times
			auto value = static_cast<int>(i * i);
			futures.push_back(CreateSleepValueFuture(sleepMillis, value));
		}

		// Act:
		auto aggregateFuture = when_all(std::move(futures));
		auto resultFutures = aggregateFuture.get();

		// Assert:
		EXPECT_TRUE(aggregateFuture.is_ready());
		AssertAllAreReady(resultFutures);

		ASSERT_EQ(Num_Futures, resultFutures.size());
		for (auto i = 0u; i < Num_Futures; ++i) {
			auto expectedValue = static_cast<int>(i * i);
			EXPECT_EQ(expectedValue, resultFutures[i].get()) << "future at " << i;
		}
	}

	// endregion

	// region compose

	TEST(TEST_CLASS, ComposeDoesNotBlock) {
		// Arrange:
		auto composedFuture = compose(CreateSleepValueFuture(25, 7), [](auto&& future) {
			return CreateSleepContinuationFuture(25, std::move(future));
		});

		// Assert:
		EXPECT_FALSE(composedFuture.is_ready());
	}

	TEST(TEST_CLASS, CanComposeNonExceptionalFutures) {
		// Arrange:
		auto composedFuture = compose(CreateSleepValueFuture(15, 7), [](auto&& future) {
			return CreateSleepContinuationFuture(10, std::move(future));
		});

		// Act:
		auto result = composedFuture.get();

		// Assert:
		EXPECT_TRUE(composedFuture.is_ready());
		EXPECT_EQ("14", result);
	}

	TEST(TEST_CLASS, CanComposeFuturesWhenFirstIsExceptional) {
		// Arrange:
		auto composedFuture = compose(CreateSleepExceptionFuture(15), [](auto&& future) {
			return CreateSleepContinuationFuture(10, std::move(future));
		});

		// Act + Assert:
		EXPECT_THROW(composedFuture.get(), catapult_runtime_error);
		EXPECT_TRUE(composedFuture.is_ready());
	}

	TEST(TEST_CLASS, CanComposeFuturesWhenSecondFutureCannotBeCreated) {
		// Arrange:
		struct Functions {
			static future<std::string> FailToCreateSecondFuture(future<int>&&) {
				throw catapult_runtime_error("failed to create second future");
			}
		};
		auto composedFuture = compose(CreateSleepValueFuture(15, 7), Functions::FailToCreateSecondFuture);

		// Act + Assert:
		EXPECT_THROW(composedFuture.get(), catapult_runtime_error);
		EXPECT_TRUE(composedFuture.is_ready());
	}

	TEST(TEST_CLASS, CanComposeFuturesWhenSecondIsExceptional) {
		// Arrange:
		auto composedFuture = compose(CreateSleepValueFuture(15, 7), [](const auto&) {
			return CreateSleepExceptionFuture(10);
		});

		// Act + Assert:
		EXPECT_THROW(composedFuture.get(), catapult_runtime_error);
		EXPECT_TRUE(composedFuture.is_ready());
	}

	TEST(TEST_CLASS, ComposeMovesFutureIntoSecondFutureFactoryFunction) {
		// Arrange:
		auto pInt = std::make_unique<int>(7);
		auto pIntRaw = pInt.get();
		std::unique_ptr<int> pIntRawFromContinuation;

		// Act
		auto composedFuture = compose(make_ready_future(std::move(pInt)), [&pIntRawFromContinuation](auto&& future) {
			pIntRawFromContinuation = future.get();
			return make_ready_future(true);
		});
		auto result = composedFuture.get();

		// Assert: the composed future completed
		EXPECT_TRUE(composedFuture.is_ready());
		EXPECT_TRUE(result);

		// - the original future data was moved into the factory function
		EXPECT_EQ(pIntRaw, pIntRawFromContinuation.get());
		EXPECT_EQ(7, *pIntRawFromContinuation);
	}

	// endregion

	// region get_all_ignore_exceptional

	TEST(TEST_CLASS, GetAllIgnoreExceptionalWorksWithZeroFutures) {
		// Arrange:
		IntFuturesVector futures;

		// Act:
		auto results = get_all_ignore_exceptional(std::move(futures));

		// Assert:
		EXPECT_TRUE(results.empty());
	}

	TEST(TEST_CLASS, GetAllIgnoreExceptionalWorksWithSingleNonExceptionalFuture) {
		// Arrange:
		IntFuturesVector futures;
		futures.push_back(CreateSleepValueFuture(15, 7));

		// Act:
		auto results = get_all_ignore_exceptional(std::move(futures));

		// Assert:
		decltype(results) expected{ 7 };
		EXPECT_EQ(expected, results);
	}

	TEST(TEST_CLASS, GetAllIgnoreExceptionalWorksWithSingleExceptionalFuture) {
		// Arrange:
		IntFuturesVector futures;
		futures.push_back(CreateSleepExceptionFuture(10));

		// Act:
		auto results = get_all_ignore_exceptional(std::move(futures));

		// Assert:
		EXPECT_TRUE(results.empty());
	}

	TEST(TEST_CLASS, GetAllIgnoreExceptionalWorksWithMultipleFutures) {
		// Arrange:
		IntFuturesVector futures;
		futures.push_back(CreateSleepValueFuture(3, 1));
		futures.push_back(CreateSleepExceptionFuture(4));
		futures.push_back(CreateSleepExceptionFuture(2));
		futures.push_back(CreateSleepValueFuture(5, 9));
		futures.push_back(CreateSleepValueFuture(7, 4));
		futures.push_back(CreateSleepExceptionFuture(2));
		futures.push_back(CreateSleepValueFuture(1, 2));

		// Act:
		auto results = get_all_ignore_exceptional(std::move(futures));

		// Assert:
		decltype(results) expected{ 1, 9, 4, 2 };
		EXPECT_EQ(expected, results);
	}

	// endregion

	// region get_all

	TEST(TEST_CLASS, GetAllWorksWithZeroFutures) {
		// Arrange:
		IntFuturesVector futures;

		// Act:
		auto results = get_all(std::move(futures));

		// Assert:
		EXPECT_TRUE(results.empty());
	}

	TEST(TEST_CLASS, GetAllWorksWithSingleNonExceptionalFuture) {
		// Arrange:
		IntFuturesVector futures;
		futures.push_back(CreateSleepValueFuture(15, 7));

		// Act:
		auto results = get_all(std::move(futures));

		// Assert:
		decltype(results) expected{ 7 };
		EXPECT_EQ(expected, results);
	}

	TEST(TEST_CLASS, GetAllWorksWithMultipleNonExceptionalFutures) {
		// Arrange:
		IntFuturesVector futures;
		futures.push_back(CreateSleepValueFuture(3, 1));
		futures.push_back(CreateSleepValueFuture(5, 9));
		futures.push_back(CreateSleepValueFuture(7, 4));
		futures.push_back(CreateSleepValueFuture(1, 2));

		// Act:
		auto results = get_all(std::move(futures));

		// Assert:
		decltype(results) expected{ 1, 9, 4, 2 };
		EXPECT_EQ(expected, results);
	}

	TEST(TEST_CLASS, GetAllThrowsWhenAnyExceptionalFutureIsEncountered) {
		// Arrange:
		IntFuturesVector futures;
		futures.push_back(CreateSleepValueFuture(3, 1));
		futures.push_back(CreateSleepExceptionFuture(10));
		futures.push_back(CreateSleepValueFuture(5, 9));
		futures.push_back(CreateSleepValueFuture(7, 3));
		futures.push_back(CreateSleepExceptionFuture(5));

		// Act + Assert:
		EXPECT_THROW(get_all(std::move(futures)), catapult_runtime_error);
	}

	// endregion
}}
