#include "src/validators/Validators.h"
#include "src/model/IdGenerator.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS RootNamespaceValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(RootNamespace, ArtifactDuration(), {})

	// region duration

	namespace {
		void AssertDurationValidationResult(uint16_t duration, uint16_t maxDuration, ValidationResult expectedResult) {
			// Arrange:
			auto pValidator = CreateRootNamespaceValidator(ArtifactDuration(maxDuration), {});
			auto notification = model::RootNamespaceNotification(Key(), NamespaceId(), ArtifactDuration(duration));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "duration " << duration << ", maxDuration " << maxDuration;
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingRootNamespaceWithDurationLessThanMax) {
		// Assert:
		AssertDurationValidationResult(12312, 12345, ValidationResult::Success);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingRootNamespaceWithDurationEqualToMax) {
		// Assert:
		AssertDurationValidationResult(12345, 12345, ValidationResult::Success);
	}

	TEST(TEST_CLASS, FailureWhenValidatingRootNamespaceWithDurationGreaterThanMax) {
		// Assert:
		AssertDurationValidationResult(12346, 12345, Failure_Namespace_Invalid_Duration);
		AssertDurationValidationResult(65432, 12345, Failure_Namespace_Invalid_Duration);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingRootNamespaceWithZeroDuration) {
		// Assert: eternal duration is allowed
		AssertDurationValidationResult(0, 12345, ValidationResult::Success);
	}

	// endregion

	// region reserved root name check

	namespace {
		void AssertReservedRootNameValidationResult(const std::string& name, ValidationResult expectedResult) {
			// Arrange:
			auto pValidator = CreateRootNamespaceValidator(
					ArtifactDuration(std::numeric_limits<ArtifactDuration::ValueType>::max()),
					{ "alpha", "beta", "gamma" });
			auto namespaceId = model::GenerateNamespaceId(Namespace_Base_Id, name);
			auto notification = model::RootNamespaceNotification(Key(), namespaceId, ArtifactDuration());

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "namespace with name" << name;
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingRootNamespaceWithNonReservedName) {
		// Assert:
		for (const auto& name : { "sigma", "be", "alpha-beta" })
			AssertReservedRootNameValidationResult(name, ValidationResult::Success);
	}

	TEST(TEST_CLASS, FailureWhenValidatingRootNamespaceWithReservedName) {
		// Assert:
		for (const auto& name : { "alpha", "beta", "gamma" })
			AssertReservedRootNameValidationResult(name, Failure_Namespace_Root_Name_Reserved);
	}

	// endregion
}}
