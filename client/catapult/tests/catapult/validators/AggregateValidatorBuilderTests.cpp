#include "catapult/validators/AggregateValidatorBuilder.h"
#include "catapult/cache/CatapultCache.h"
#include "tests/catapult/validators/utils/AggregateValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AggregateValidatorBuilderTests

	namespace {
		using AggregateNotificationValidator = AggregateNotificationValidatorT<test::TaggedNotification, const ValidatorContext&>;

		struct TestContext {
		public:
			std::vector<uint16_t> Breadcrumbs;
			std::unique_ptr<const AggregateNotificationValidator> pAggregateValidator;

		public:
			ValidationResult validate(uint8_t notificationId) {
				auto cache = test::CreateEmptyCatapultCache();
				auto cacheView = cache.createView();
				auto context = test::CreateValidatorContext(Height(123), cacheView.toReadOnly());
				return test::ValidateNotification(*pAggregateValidator, test::TaggedNotification(notificationId), context);
			}
		};

		std::unique_ptr<TestContext> CreateTestContext(const std::vector<ValidationResult>& results) {
			auto pContext = std::make_unique<TestContext>();
			AggregateValidatorBuilder<test::TaggedNotification, const ValidatorContext&> builder;
			for (auto i = 0u; i < results.size(); ++i)
				builder.add(mocks::CreateTaggedBreadcrumbValidator(static_cast<uint8_t>(i + 1), pContext->Breadcrumbs, results[i]));

			pContext->pAggregateValidator = builder.build();
			return pContext;
		}

		std::unique_ptr<TestContext> CreateTestContext(size_t numValidators) {
			return CreateTestContext(std::vector<ValidationResult>(numValidators, ValidationResult::Success));
		}
	}

	// region basic delegation

	TEST(TEST_CLASS, CanCreateEmptyAggregateValidator) {
		// Act:
		auto pContext = CreateTestContext(0);
		auto result = pContext->validate(7);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);

		std::vector<std::string> expectedNames;
		EXPECT_EQ(0u, pContext->Breadcrumbs.size());
		EXPECT_EQ("{}", pContext->pAggregateValidator->name());
		EXPECT_EQ(expectedNames, pContext->pAggregateValidator->names());
	}

	TEST(TEST_CLASS, CanCreateAggregateValidatorWithMultipleValidators) {
		// Act:
		auto pContext = CreateTestContext(10);
		auto result = pContext->validate(7);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);

		std::vector<std::string> expectedNames{ "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" };
		EXPECT_EQ(10u, pContext->Breadcrumbs.size());
		EXPECT_EQ("{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }", pContext->pAggregateValidator->name());
		EXPECT_EQ(expectedNames, pContext->pAggregateValidator->names());
	}

	TEST(TEST_CLASS, AddAllowsChaining) {
		// Act:
		auto pContext = std::make_unique<TestContext>();
		AggregateValidatorBuilder<test::TaggedNotification, const ValidatorContext&> builder;
		builder
			.add(mocks::CreateTaggedBreadcrumbValidator(2, pContext->Breadcrumbs))
			.add(mocks::CreateTaggedBreadcrumbValidator(3, pContext->Breadcrumbs))
			.add(mocks::CreateTaggedBreadcrumbValidator(4, pContext->Breadcrumbs));
		pContext->pAggregateValidator = builder.build();

		// Act:
		auto result = pContext->validate(7);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);

		std::vector<uint16_t> expectedBreadcrumbs{ 0x0702, 0x0703, 0x0704 };
		EXPECT_EQ(3u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	// endregion

	// region validate

	TEST(TEST_CLASS, AggregateValidatorForwardsToAllValidatorsOnSuccess) {
		// Act:
		auto pContext = CreateTestContext(5);
		auto result = pContext->validate(7);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);

		std::vector<uint16_t> expectedBreadcrumbs{ 0x0701, 0x0702, 0x0703, 0x0704, 0x0705 };
		EXPECT_EQ(5u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	TEST(TEST_CLASS, AggregateValidatorCanPerformMultipleValidations) {
		// Act:
		auto pContext = CreateTestContext(3);
		pContext->validate(2);
		pContext->validate(7);
		pContext->validate(5);
		pContext->validate(1);

		// Assert:
		std::vector<uint16_t> expectedBreadcrumbs{
			0x0201, 0x0202, 0x0203,
			0x0701, 0x0702, 0x0703,
			0x0501, 0x0502, 0x0503,
			0x0101, 0x0102, 0x0103,
		};
		EXPECT_EQ(12u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	// endregion

	// region forwarding

	TEST(TEST_CLASS, NotificationsAreForwardedToChildValidators) {
		// Assert:
		AssertNotificationsAreForwardedToChildValidators(
				AggregateValidatorBuilder<model::Notification, const ValidatorContext&>(),
				[](auto& builder, auto&& pValidator) { builder.add(std::move(pValidator)); });
	}

	TEST(TEST_CLASS, ContextsAreForwardedToChildValidators) {
		// Assert:
		AssertContextsAreForwardedToChildValidators(
				AggregateValidatorBuilder<model::Notification, const ValidatorContext&>(),
				[](auto& builder, auto&& pValidator) { builder.add(std::move(pValidator)); });
	}

	// endregion

	// region result aggregation / short-circuiting

	TEST(TEST_CLASS, NeutralResultDominatesSuccessResult) {
		// Act:
		auto pContext = CreateTestContext({ ValidationResult::Success, ValidationResult::Neutral, ValidationResult::Success });
		auto result = pContext->validate(7);

		// Assert:
		EXPECT_EQ(ValidationResult::Neutral, result);

		std::vector<uint16_t> expectedBreadcrumbs{ 0x0701, 0x0702, 0x0703 };
		EXPECT_EQ(3u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	TEST(TEST_CLASS, FailureResultDominatesSuccessResult) {
		// Act:
		auto pContext = CreateTestContext({ ValidationResult::Success, ValidationResult::Neutral, ValidationResult::Failure });
		auto result = pContext->validate(7);

		// Assert:
		EXPECT_EQ(ValidationResult::Failure, result);

		std::vector<uint16_t> expectedBreadcrumbs{ 0x0701, 0x0702, 0x0703 };
		EXPECT_EQ(3u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	TEST(TEST_CLASS, FailureShortCircuitsSubsequentValidations) {
		// Act:
		auto pContext = CreateTestContext({ ValidationResult::Failure, ValidationResult::Success, ValidationResult::Neutral });
		auto result = pContext->validate(7);

		// Assert: only the first validator was called before short-circuiting
		EXPECT_EQ(ValidationResult::Failure, result);

		std::vector<uint16_t> expectedBreadcrumbs{ 0x0701 };
		EXPECT_EQ(1u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	// endregion
}}
