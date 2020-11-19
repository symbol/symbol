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
			[&breadcrumbs](const auto& sentinel) { breadcrumbs.push_back(&sentinel); }
		});

		ASSERT_TRUE(!!consumer);

		// - call the consumer
		int sentinel = 11;
		consumer(sentinel);

		// Assert:
		ASSERT_EQ(1u, breadcrumbs.size());
		EXPECT_EQ(&sentinel, breadcrumbs[0]);
	}

	TEST(TEST_CLASS, CanAggregateMultipleConsumers) {
		// Arrange:
		struct Breadcrumb { const int* pData; size_t Id; };
		std::vector<Breadcrumb> breadcrumbs;

		// Act:
		auto consumer = AggregateConsumers<catapult::consumer<const int&>>({
			[&breadcrumbs](const auto& sentinel) { breadcrumbs.push_back({ &sentinel, 1 }); },
			[&breadcrumbs](const auto& sentinel) { breadcrumbs.push_back({ &sentinel, 2 }); }
		});

		ASSERT_TRUE(!!consumer);

		// - call the consumer
		int sentinel = 11;
		consumer(sentinel);

		// Assert:
		ASSERT_EQ(2u, breadcrumbs.size());
		for (auto i = 0u; i < breadcrumbs.size(); ++i) {
			EXPECT_EQ(&sentinel, breadcrumbs[i].pData) << "at " << i;
			EXPECT_EQ(i + 1u, breadcrumbs[i].Id) << "at " << i;
		}
	}

	// endregion
}}
