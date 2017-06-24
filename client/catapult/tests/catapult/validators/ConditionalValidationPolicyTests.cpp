#include "catapult/validators/ConditionalValidationPolicy.h"
#include "tests/catapult/validators/utils/ValidationPolicyTestUtils.h"
#include "tests/test/other/mocks/MockDispatchFunctions.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	namespace {
		struct TestContext {
		public:
			TestContext(uint32_t threshold, uint32_t numEntities, uint32_t numValidationFunctions)
					: ConditionalDispatchFunc(CreateConditionalValidationPolicy(
							mocks::Wrap(SequentialDispatchFunc),
							mocks::Wrap(ParallelDispatchFunc),
							threshold))
					, EntityInfos(numEntities) {
				// Arrange: configure dispatch functions to return different results
				SequentialDispatchFunc.setResult(ValidationResult::Neutral);
				ParallelDispatchFunc.setResult(ValidationResult::Failure);

				// - create a few noop validation functions
				for (auto i = 0u; i < numValidationFunctions; ++i)
					ValidationFunctions.push_back([](const auto&) { return ValidationResult::Success; });
			}

		public:
			mocks::MockSequentialDispatchFunc SequentialDispatchFunc;
			mocks::MockParallelDispatchFunc ParallelDispatchFunc;
			SequentialValidationPolicyFunc ConditionalDispatchFunc;

			test::EntityInfoContainerWrapper EntityInfos;
			validators::ValidationFunctions ValidationFunctions;
		};

		void AssertSequentialDelegation(uint32_t threshold, uint32_t numEntities) {
			// Arrange:
			TestContext context(threshold, numEntities, 3);

			// Act:
			auto result = context.ConditionalDispatchFunc(context.EntityInfos.toVector(), context.ValidationFunctions);

			// Assert: the sequential dispatcher should have been used
			const auto& sequentialParams = context.SequentialDispatchFunc.params();
			const auto& parallelParams = context.ParallelDispatchFunc.params();
			EXPECT_EQ(ValidationResult::Neutral, result);
			ASSERT_EQ(1u, sequentialParams.size());
			EXPECT_EQ(0u, parallelParams.size());

			EXPECT_EQ(context.EntityInfos.toVector(), sequentialParams[0].EntityInfos);
			EXPECT_EQ(3u, sequentialParams[0].NumValidationFunctions);
		}

		void AssertParallelDelegation(uint32_t threshold, uint32_t numEntities) {
			// Arrange:
			TestContext context(threshold, numEntities, 3);

			// Act:
			auto result = context.ConditionalDispatchFunc(context.EntityInfos.toVector(), context.ValidationFunctions);

			// Assert: the parallel dispatcher should have been used
			const auto& sequentialParams = context.SequentialDispatchFunc.params();
			const auto& parallelParams = context.ParallelDispatchFunc.params();
			EXPECT_EQ(ValidationResult::Failure, result);
			EXPECT_EQ(0u, sequentialParams.size());
			ASSERT_EQ(1u, parallelParams.size());

			EXPECT_EQ(context.EntityInfos.toVector(), parallelParams[0].EntityInfos);
			EXPECT_EQ(3u, parallelParams[0].NumValidationFunctions);
		}
	}

	TEST(ConditionalValidationPolicyTests, DelegatesToSequentialWhenThresholdIsGreaterThanNumEntities) {
		// Assert:
		AssertSequentialDelegation(5, 4);
	}

	TEST(ConditionalValidationPolicyTests, DelegatesToSequentialWhenThresholdIsEqualToNumEntities) {
		// Assert:
		AssertSequentialDelegation(5, 5);
	}

	TEST(ConditionalValidationPolicyTests, DelegatesToParallelWhenThresholdIsLessThanNumEntities) {
		// Assert:
		AssertParallelDelegation(5, 6);
	}
}}
