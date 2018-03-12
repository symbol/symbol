#include "catapult/extensions/BasicServerHooks.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS BasicServerHooksTests

	using BasicFunc = supplier<int>;

	// region SetOnce

	TEST(TEST_CLASS, SetOnceSetsDestWhenDestIsUnset) {
		// Arrange:
		BasicFunc dest;
		BasicFunc source = []() { return 123; };

		// Act:
		SetOnce(dest, source);

		// Assert:
		ASSERT_TRUE(!!dest);
		EXPECT_EQ(123, dest());
	}

	TEST(TEST_CLASS, SetOnceThrowsWhenDestIsSet) {
		// Arrange:
		BasicFunc dest = []() { return 123; };
		BasicFunc source = []() { return 987; };

		// Act + Assert:
		EXPECT_THROW(SetOnce(dest, source), catapult_invalid_argument);

		// Assert:
		ASSERT_TRUE(!!dest);
		EXPECT_EQ(123, dest());
	}

	// endregion

	// region Require

	TEST(TEST_CLASS, RequireReturnsFuncWhenFuncIsSet) {
		// Act:
		auto func = Require(BasicFunc([]() { return 123; }));

		// Assert:
		ASSERT_TRUE(!!func);
		EXPECT_EQ(123, func());
	}

	TEST(TEST_CLASS, RequireThrowsWhenFuncIsNotSet) {
		// Act + Assert:
		EXPECT_THROW(Require(BasicFunc()), catapult_invalid_argument);
	}

	// endregion

	// region AggregateConsumers

	TEST(TEST_CLASS, CanAggregateZeroConsumers) {
		// Act:
		auto consumer = AggregateConsumers<catapult::consumer<const int&>>({});
		ASSERT_TRUE(!!consumer);

		// Assert: no exception
		consumer(7);
	}

	TEST(TEST_CLASS, CanAggregateSingleConsumer) {
		// Arrange:
		std::vector<const int*> breadcrumbs;

		// Act:
		auto consumer = AggregateConsumers<catapult::consumer<const int&>>({
			[&breadcrumbs](const auto& data) { breadcrumbs.push_back(&data); }
		});

		ASSERT_TRUE(!!consumer);

		// - call the consumer
		int data = 11;
		consumer(data);

		// Assert:
		ASSERT_EQ(1u, breadcrumbs.size());
		EXPECT_EQ(&data, breadcrumbs[0]);
	}

	TEST(TEST_CLASS, CanAggregateMultipleConsumers) {
		// Arrange:
		struct Breadcrumb { const int* pData; size_t Id; };
		std::vector<Breadcrumb> breadcrumbs;

		// Act:
		auto consumer = AggregateConsumers<catapult::consumer<const int&>>({
			[&breadcrumbs](const auto& data) { breadcrumbs.push_back({ &data, 1 }); },
			[&breadcrumbs](const auto& data) { breadcrumbs.push_back({ &data, 2 }); }
		});

		ASSERT_TRUE(!!consumer);

		// - call the consumer
		int data = 11;
		consumer(data);

		// Assert:
		ASSERT_EQ(2u, breadcrumbs.size());
		for (auto i = 0u; i < breadcrumbs.size(); ++i) {
			EXPECT_EQ(&data, breadcrumbs[i].pData) << "at " << i;
			EXPECT_EQ(i + 1u, breadcrumbs[i].Id) << "at " << i;
		}
	}

	// endregion
}}
