#include "src/validators/Validators.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS NamespaceTypeValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(NamespaceType,)

	// region namespace type

	namespace {
		void AssertNamespaceTypeResult(ValidationResult expectedResult, model::NamespaceType namespaceType) {
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
			AssertNamespaceTypeResult(ValidationResult::Success, static_cast<model::NamespaceType>(namespaceType));
	}

	TEST(TEST_CLASS, FailureWhenValidatingInvalidNamespaceType) {
		// Assert:
		for (auto namespaceType : { 0x02, 0xFF })
			AssertNamespaceTypeResult(Failure_Namespace_Invalid_Namespace_Type, static_cast<model::NamespaceType>(namespaceType));
	}

	// endregion
}}
