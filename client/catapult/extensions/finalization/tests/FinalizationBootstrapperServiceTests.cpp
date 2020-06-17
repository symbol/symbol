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

#include "finalization/src/FinalizationBootstrapperService.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace finalization {

#define TEST_CLASS FinalizationBootstrapperServiceTests

	// region FinalizationServerHooks

	namespace {
		struct MessageRangeConsumerTraits {
			using ParamType = model::AnnotatedEntityRange<model::FinalizationMessage>;

			static auto Get(const FinalizationServerHooks& hooks) {
				return hooks.messageRangeConsumer();
			}

			static void Set(FinalizationServerHooks& hooks, const handlers::RangeHandler<model::FinalizationMessage>& consumer) {
				hooks.setMessageRangeConsumer(consumer);
			}
		};
	}

#define CONSUMER_HOOK_TEST_ENTRY(TEST_NAME, CONSUMER_FACTORY_NAME) \
	TEST(TEST_CLASS, Hooks_##TEST_NAME##_##CONSUMER_FACTORY_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CONSUMER_FACTORY_NAME##Traits>(); \
	}

#define CONSUMER_HOOK_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	CONSUMER_HOOK_TEST_ENTRY(TEST_NAME, MessageRangeConsumer) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	CONSUMER_HOOK_TEST(CannotAccessWhenUnset) {
		// Arrange:
		FinalizationServerHooks hooks;

		// Act + Assert:
		EXPECT_THROW(TTraits::Get(hooks), catapult_invalid_argument);
	}

	CONSUMER_HOOK_TEST(CanSetOnce) {
		// Arrange:
		FinalizationServerHooks hooks;

		typename TTraits::ParamType seedParam;
		const auto* pSeedParam = &seedParam;
		std::vector<decltype(pSeedParam)> consumedParams;

		TTraits::Set(hooks, [&consumedParams](auto&& param) {
			consumedParams.push_back(&param);
		});

		// Act:
		auto factory = TTraits::Get(hooks);
		ASSERT_TRUE(!!factory);

		factory(std::move(seedParam));

		// Assert: the param created above should be passed (and moved) down
		ASSERT_EQ(1u, consumedParams.size());
		EXPECT_EQ(pSeedParam, consumedParams[0]);
	}

	CONSUMER_HOOK_TEST(CannotSetMultipleTimes) {
		// Arrange:
		FinalizationServerHooks hooks;
		TTraits::Set(hooks, [](auto&&) {});

		// Act + Assert:
		EXPECT_THROW(TTraits::Set(hooks, [](auto&&) {}), catapult_invalid_argument);
	}

	// endregion

	namespace {
		struct FinalizationBootstrapperServiceTraits {
			static constexpr auto CreateRegistrar = CreateFinalizationBootstrapperServiceRegistrar;
		};

		using TestContext = test::ServiceLocatorTestContext<FinalizationBootstrapperServiceTraits>;
	}

	// region FinalizationBootstrapperService basic

	ADD_SERVICE_REGISTRAR_INFO_TEST(FinalizationBootstrapper, Initial)

	TEST(TEST_CLASS, NoCountersAreRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(0u, context.locator().counters().size());
	}

	TEST(TEST_CLASS, FinalizationHooksServiceIsRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(1u, context.locator().numServices());

		// - service (get does not throw)
		GetFinalizationServerHooks(context.locator());
	}

	// endregion
}}
