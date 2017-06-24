#include "src/validators/Validators.h"
#include "src/model/IdGenerator.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS NamespaceNameValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(NamespaceName, 0)

	namespace {
		model::NamespaceNameNotification CreateNamespaceNameNotification(uint8_t nameSize, const uint8_t* pName) {
			auto notification = model::NamespaceNameNotification(NamespaceId(), NamespaceId(777), nameSize, pName);
			notification.NamespaceId = model::GenerateNamespaceId(NamespaceId(777), reinterpret_cast<const char*>(pName));
			return notification;
		}
	}

	// region name size

	namespace {
		void AssertSizeValidationResult(uint8_t nameSize, uint8_t maxNameSize, ValidationResult expectedResult) {
			// Arrange:
			auto pValidator = CreateNamespaceNameValidator(maxNameSize);
			auto name = std::string(nameSize, 'a');
			auto notification = CreateNamespaceNameNotification(nameSize, reinterpret_cast<const uint8_t*>(name.data()));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result)
					<< "nameSize " << static_cast<uint16_t>(nameSize)
					<< ", maxNameSize " << static_cast<uint16_t>(maxNameSize);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNamespaceWithNameSizeLessThanMax) {
		// Assert:
		AssertSizeValidationResult(100, 123, ValidationResult::Success);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNamespaceWithNameSizeEqualToMax) {
		// Assert:
		AssertSizeValidationResult(123, 123, ValidationResult::Success);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNamespaceWithNameSizeGreaterThanMax) {
		// Assert:
		AssertSizeValidationResult(124, 123, Failure_Namespace_Invalid_Name);
		AssertSizeValidationResult(200, 123, Failure_Namespace_Invalid_Name);
	}

	TEST(TEST_CLASS, FailureWhenValidatingEmptyNamespaceName) {
		// Assert:
		AssertSizeValidationResult(0, 123, Failure_Namespace_Invalid_Name);
	}

	// endregion

	// region name characters

	namespace {
		void AssertNameValidationResult(const std::string& name, ValidationResult expectedResult) {
			// Arrange:
			auto pValidator = CreateNamespaceNameValidator(static_cast<uint8_t>(name.size()));
			auto notification = CreateNamespaceNameNotification(
					static_cast<uint8_t>(name.size()),
					reinterpret_cast<const uint8_t*>(name.data()));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "namespace with name" << name;
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingValidNamespaceNames) {
		// Assert:
		for (const auto& name : { "a", "be", "cat", "doom", "al-ce", "al_ce", "alice-", "alice_" })
			AssertNameValidationResult(name, ValidationResult::Success);
	}

	TEST(TEST_CLASS, FailureWhenValidatingInvalidNamespaceNames) {
		// Assert:
		for (const auto& name : { "-alice", "_alice", "al.ce", "alIce", "al ce", "al@ce", "al#ce", "!@#$%" })
			AssertNameValidationResult(name, Failure_Namespace_Invalid_Name);
	}

	// endregion

	// region name and id consistency

	TEST(TEST_CLASS, SuccessWhenValidatingNamespaceWithMatchingNameAndId) {
		// Arrange: note that CreateNamespaceNameNotification creates proper id
		auto pValidator = CreateNamespaceNameValidator(100);
		auto name = std::string(10, 'a');
		auto notification = CreateNamespaceNameNotification(
				static_cast<uint8_t>(name.size()),
				reinterpret_cast<const uint8_t*>(name.data()));

		// Act:
		auto result = test::ValidateNotification(*pValidator, notification);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNamespaceWithMismatchedNameAndId) {
		// Arrange: corrupt the id
		auto pValidator = CreateNamespaceNameValidator(100);
		auto name = std::string(10, 'a');
		auto notification = CreateNamespaceNameNotification(
				static_cast<uint8_t>(name.size()),
				reinterpret_cast<const uint8_t*>(name.data()));
		notification.NamespaceId = notification.NamespaceId + NamespaceId(1);

		// Act:
		auto result = test::ValidateNotification(*pValidator, notification);

		// Assert:
		EXPECT_EQ(Failure_Namespace_Name_Id_Mismatch, result);
	}

	// endregion
}}
