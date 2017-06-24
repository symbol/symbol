#include "catapult/validators/SequentialValidationPolicy.h"
#include "catapult/validators/ValidatorContext.h"
#include "tests/catapult/validators/utils/MockEntityValidator.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace validators {

	#define TEST_CLASS SequentialValidationPolicyTests

	namespace {
		struct SequentialValidationPolicyTraits {
			using PairVector = std::vector<std::pair<std::string, ValidationResult>>;
			using ValidatorType = mocks::MockEntityValidator<>;
			using ValidatorStateVector = std::vector<std::shared_ptr<mocks::SingleThreadedValidatorState>>;

			static auto CreateDispatcher() {
				return CreateSequentialValidationPolicy();
			}

			template<typename TValidator, typename TEntityInfos>
			static ValidationResult Dispatch(
					const TValidator& validator,
					const SequentialValidationPolicyFunc& dispatcher,
					const TEntityInfos& entityInfos) {
				mocks::EmptyValidatorContext context;
				return validator.curry(context.cref()).dispatch(dispatcher, entityInfos);
			}

			template<typename TValidator>
			static ValidationResult Dispatch(
					const TValidator& validator,
					const SequentialValidationPolicyFunc& dispatcher,
					const model::WeakEntityInfos& entityInfos,
					const ValidatorContext& context) {
				return validator.curry(std::cref(context)).dispatch(dispatcher, entityInfos);
			}
		};
	}
}}

#define AGGREGATE_VALIDATOR_VALIDATION_POLICY_TEST(TEST_NAME) \
		template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
		TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<SequentialValidationPolicyTraits>(); } \
		template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#include "tests/catapult/validators/utils/ValidationPolicyTests.h"
