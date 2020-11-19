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

#pragma once
#include <vector>

namespace catapult { namespace test {

	/// Tests for a consumer handler.
	template<typename TConsumerOwner, typename TTraits>
	class ConsumerHandlerTests {
	public:
		static void AssertCanCreateConsumerAroundZeroFunctions() {
			// Arrange:
			TConsumerOwner owner;

			// Act:
			auto consumer = TTraits::CreateConsumer(owner);
			ASSERT_TRUE(!!consumer);

			// Assert: no exception
			consumer(TTraits::CreateConsumerInput());
		}

		static void AssertCanCreateConsumerAroundMultipleFunctions() {
			// Arrange:
			struct Breadcrumb {
				const decltype(TTraits::CreateConsumerInput())* pInput;
				size_t Id;
			};

			TConsumerOwner owner;
			std::vector<Breadcrumb> breadcrumbs;
			TTraits::AddConsumer(owner, [&breadcrumbs](const auto& input) { breadcrumbs.push_back({ &input, 1 }); });
			TTraits::AddConsumer(owner, [&breadcrumbs](const auto& input) { breadcrumbs.push_back({ &input, 2 }); });
			auto consumerInput = TTraits::CreateConsumerInput();

			// Act:
			auto consumer = TTraits::CreateConsumer(owner);
			ASSERT_TRUE(!!consumer);

			consumer(consumerInput);

			// Assert:
			ASSERT_EQ(2u, breadcrumbs.size());
			for (auto i = 0u; i < breadcrumbs.size(); ++i) {
				EXPECT_EQ(&consumerInput, breadcrumbs[i].pInput) << "at " << i;
				EXPECT_EQ(i + 1u, breadcrumbs[i].Id) << "at " << i;
			}
		}
	};

#define MAKE_CONSUMER_HANDLER_TEST(TEST_CLASS, OWNER, CONSUMER_HANDLER, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##_##CONSUMER_HANDLER) { test::ConsumerHandlerTests<OWNER, CONSUMER_HANDLER##Traits>::Assert##TEST_NAME(); }

#define DEFINE_CONSUMER_HANDLER_TESTS(TEST_CLASS, OWNER, CONSUMER_HANDLER) \
	MAKE_CONSUMER_HANDLER_TEST(TEST_CLASS, OWNER, CONSUMER_HANDLER, CanCreateConsumerAroundZeroFunctions) \
	MAKE_CONSUMER_HANDLER_TEST(TEST_CLASS, OWNER, CONSUMER_HANDLER, CanCreateConsumerAroundMultipleFunctions)
}}
