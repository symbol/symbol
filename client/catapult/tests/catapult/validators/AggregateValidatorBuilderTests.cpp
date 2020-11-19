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

#include "catapult/validators/AggregateValidatorBuilder.h"
#include "catapult/cache/CatapultCache.h"
#include "tests/catapult/validators/test/AggregateValidatorTestUtils.h"
#include "tests/test/other/ValidationResultTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AggregateValidatorBuilderTests

	namespace {
		using AggregateNotificationValidator = AggregateNotificationValidatorT<test::TaggedNotification, const ValidatorContext&>;

		constexpr auto Failure1_Result = test::MakeValidationResult(ResultSeverity::Failure, 3);
		constexpr auto Failure2_Result = test::MakeValidationResult(ResultSeverity::Failure, 4);

		struct TestContext {
		public:
			std::vector<uint16_t> Breadcrumbs;
			std::unique_ptr<const AggregateNotificationValidator> pAggregateValidator;

		public:
			ValidationResult validate(uint8_t notificationId) {
				auto cache = test::CreateEmptyCatapultCache();
				return test::ValidateNotification(*pAggregateValidator, test::TaggedNotification(notificationId), cache);
			}
		};

		std::unique_ptr<TestContext> CreateTestContext(
				const std::vector<ValidationResult>& results,
				const std::set<ValidationResult>& suppressedFailures = {}) {
			auto pContext = std::make_unique<TestContext>();
			AggregateValidatorBuilder<test::TaggedNotification, const ValidatorContext&> builder;
			for (auto i = 0u; i < results.size(); ++i)
				builder.add(mocks::CreateTaggedBreadcrumbValidator(static_cast<uint8_t>(i + 1), pContext->Breadcrumbs, results[i]));

			pContext->pAggregateValidator = builder.build([failures = suppressedFailures](auto result) {
				return failures.cend() != failures.find(result);
			});
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
		pContext->pAggregateValidator = builder.build([](auto) { return false; });

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
			0x0101, 0x0102, 0x0103
		};
		EXPECT_EQ(12u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	// endregion

	// region forwarding

	TEST(TEST_CLASS, NotificationsAreForwardedToChildValidators) {
		using BuilderType = AggregateValidatorBuilder<model::Notification, const ValidatorContext&>;
		test::AssertNotificationsAreForwardedToChildValidators(BuilderType(), [](auto& builder, auto&& pValidator) {
			builder.add(std::move(pValidator));
		});
	}

	TEST(TEST_CLASS, ContextsAreForwardedToChildValidators) {
		using BuilderType = AggregateValidatorBuilder<model::Notification, const ValidatorContext&>;
		test::AssertContextsAreForwardedToChildValidators(BuilderType(), [](auto& builder, auto&& pValidator) {
			builder.add(std::move(pValidator));
		});
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

	TEST(TEST_CLASS, FailuresCanBeSuppressed) {
		// Act: create a validator with a single suppressed failure
		auto pContext = CreateTestContext({ Failure1_Result, ValidationResult::Success, ValidationResult::Neutral }, { Failure1_Result });
		auto result = pContext->validate(7);

		// Assert: all validators were called
		EXPECT_EQ(ValidationResult::Neutral, result);

		std::vector<uint16_t> expectedBreadcrumbs{ 0x0701, 0x702, 0x703 };
		EXPECT_EQ(3u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	TEST(TEST_CLASS, UnsuppressedFailureResultDominatesSuppressedFailureResult) {
		// Act: create a validator with suppressed (failure 1) and unsuppressed (failure 2) failures
		auto pContext = CreateTestContext(
				{ Failure1_Result, ValidationResult::Success, Failure2_Result, ValidationResult::Neutral },
				{ Failure1_Result });
		auto result = pContext->validate(7);

		// Assert: all validators were called until the first unsuppressed failure result
		EXPECT_EQ(Failure2_Result, result);

		std::vector<uint16_t> expectedBreadcrumbs{ 0x0701, 0x702, 0x703 };
		EXPECT_EQ(3u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	// endregion
}}
