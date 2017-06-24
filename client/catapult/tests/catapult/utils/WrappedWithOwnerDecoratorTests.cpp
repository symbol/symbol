#include "catapult/utils/WrappedWithOwnerDecorator.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS WrappedWithOwnerDecoratorTests

	namespace {
		struct BasicTraits {
			template<typename... TArgs>
			using DecoratorType = WrappedWithOwnerDecorator<TArgs...>;
		};

		struct ResettableTraits {
			template<typename... TArgs>
			using DecoratorType = ResettableWrappedWithOwnerDecorator<TArgs...>;
		};
	}

#define DECORATOR_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BasicTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Resettable) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ResettableTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	DECORATOR_BASED_TEST(DecoratorForwardsToCallable) {
		// Arrange: wrap a decorator around a lambda that depends on pOwner
		auto pOwner = std::make_shared<uint32_t>(7);
		auto counter = 0u;
		auto callable = [owner = *pOwner, &counter](uint32_t multiple) {
			counter += owner * multiple;
		};
		typename TTraits::template DecoratorType<decltype(callable)> decorator(callable, pOwner);

		// Act: invoke the lambda
		decorator(3u);
		decorator(2u);

		// Assert: the function was called and decorator has kept the owner alive
		EXPECT_EQ(35u, counter);
		EXPECT_EQ(2u, decorator.owner().use_count());
	}

	DECORATOR_BASED_TEST(DecoratorForwardsResultsFromCallabale) {
		// Arrange: wrap a decorator around a lambda that depends on pOwner
		auto pOwner = std::make_shared<uint32_t>(7);
		auto callable = [owner = *pOwner](uint32_t multiple) {
			return owner * multiple;
		};
		typename TTraits::template DecoratorType<decltype(callable)> decorator(callable, pOwner);

		// Act: invoke the lambda
		auto result1 = decorator(3u);
		auto result2 = decorator(2u);

		// Assert: the function was called and decorator has kept the owner alive
		EXPECT_EQ(21u, result1);
		EXPECT_EQ(14u, result2);
		EXPECT_EQ(2u, decorator.owner().use_count());
	}

	DECORATOR_BASED_TEST(DecoratorExtendsLifetime) {
		// Arrange: wrap a decorator around a lambda that depends on pOwner
		auto pOwner = std::make_shared<uint32_t>(7);
		auto counter = 0u;
		auto callable = [owner = *pOwner, &counter](uint32_t multiple) {
			counter += owner * multiple;
		};
		typename TTraits::template DecoratorType<decltype(callable)> decorator(callable, pOwner);

		// Act: reset the local owner pointer
		EXPECT_EQ(2u, decorator.owner().use_count());
		pOwner.reset();
		EXPECT_EQ(1u, decorator.owner().use_count());

		// - invoke the lambda
		decorator(3u);

		// Assert: the function was called and decorator has kept the owner alive
		EXPECT_EQ(21u, counter);
	}

	TEST(WrappedWithOwnerDecorator, ResettableDecoratorCanBeReset) {
		// Arrange: wrap a decorator around a lambda that depends on pOwner
		auto pOwner = std::make_shared<uint32_t>(7);
		auto counter = 0u;
		ResettableWrappedWithOwnerDecorator<std::function<void (uint32_t)>> decorator([pOwner, &counter](
				uint32_t multiple) {
			counter += *pOwner * multiple;
		}, pOwner);

		// Sanity: local, captured owner in lambda, captured owner in decorator
		EXPECT_EQ(3u, pOwner.use_count());

		// Act: reset the decorator:
		decorator.reset();

		// Assert: only the local reference remains
		EXPECT_EQ(1u, pOwner.use_count());
	}
}}
