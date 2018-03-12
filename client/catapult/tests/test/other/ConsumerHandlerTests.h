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
			consumer(TTraits::CreateConsumerData());
		}

		static void AssertCanCreateConsumerAroundMultipleFunctions() {
			// Arrange:
			struct Breadcrumb {
				const decltype(TTraits::CreateConsumerData())* pData;
				size_t Id;
			};

			TConsumerOwner owner;
			std::vector<Breadcrumb> breadcrumbs;
			TTraits::AddConsumer(owner, [&breadcrumbs](const auto& data) { breadcrumbs.push_back({ &data, 1 }); });
			TTraits::AddConsumer(owner, [&breadcrumbs](const auto& data) { breadcrumbs.push_back({ &data, 2 }); });
			auto data = TTraits::CreateConsumerData();

			// Act:
			auto consumer = TTraits::CreateConsumer(owner);
			ASSERT_TRUE(!!consumer);

			consumer(data);

			// Assert:
			ASSERT_EQ(2u, breadcrumbs.size());
			for (auto i = 0u; i < breadcrumbs.size(); ++i) {
				EXPECT_EQ(&data, breadcrumbs[i].pData) << "at " << i;
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
