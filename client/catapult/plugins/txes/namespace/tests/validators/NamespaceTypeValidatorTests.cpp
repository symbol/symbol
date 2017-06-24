#include "src/validators/Validators.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS NamespaceTypeValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(NamespaceType,)

	// region namespace type

	namespace {
		void AssertNamespaceTypeResult(model::NamespaceType namespaceType, ValidationResult expectedResult) {
			// Arrange:
			auto pValidator = CreateNamespaceTypeValidator();
			auto notification = model::NamespaceNotification(namespaceType);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "namespaceType " << static_cast<uint16_t>(namespaceType);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingValidNamespaceType) {
		// Assert:
		for (auto namespaceType : { 0x00, 0x01 })
			AssertNamespaceTypeResult(static_cast<model::NamespaceType>(namespaceType), ValidationResult::Success);
	}

	TEST(TEST_CLASS, FailureWhenValidatingInvalidNamespaceType) {
		// Assert:
		for (auto namespaceType : { 0x02, 0xFF })
			AssertNamespaceTypeResult(static_cast<model::NamespaceType>(namespaceType), Failure_Namespace_Invalid_Namespace_Type);
	}

	// endregion
}}
