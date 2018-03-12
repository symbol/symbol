#include "src/validators/Validators.h"
#include "src/model/IdGenerator.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicNameValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicName, 0)

	namespace {
		model::MosaicNameNotification CreateMosaicNameNotification(uint8_t nameSize, const uint8_t* pName) {
			auto notification = model::MosaicNameNotification(MosaicId(), NamespaceId(777), nameSize, pName);
			notification.MosaicId = model::GenerateMosaicId(NamespaceId(777), reinterpret_cast<const char*>(pName));
			return notification;
		}
	}

	// region reserved ids

	TEST(TEST_CLASS, FailureWhenMosaicNameIsReserved) {
		// Arrange:
		auto pValidator = CreateMosaicNameValidator(100);
		auto name = std::string(10, 'a');
		auto notification = CreateMosaicNameNotification(static_cast<uint8_t>(name.size()), reinterpret_cast<const uint8_t*>(name.data()));
		notification.MosaicId = MosaicId();

		// Act:
		auto result = test::ValidateNotification(*pValidator, notification);

		// Assert:
		EXPECT_EQ(Failure_Mosaic_Name_Reserved, result);
	}

	// endregion

	// region name size

	namespace {
		void AssertSizeValidationResult(ValidationResult expectedResult, uint8_t nameSize, uint8_t maxNameSize) {
			// Arrange:
			auto pValidator = CreateMosaicNameValidator(maxNameSize);
			auto name = std::string(nameSize, 'a');
			auto notification = CreateMosaicNameNotification(nameSize, reinterpret_cast<const uint8_t*>(name.data()));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result)
					<< "nameSize " << static_cast<uint16_t>(nameSize)
					<< ", maxNameSize " << static_cast<uint16_t>(maxNameSize);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingMosaicWithNameSizeLessThanMax) {
		// Assert:
		AssertSizeValidationResult(ValidationResult::Success, 100, 123);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingMosaicWithNameSizeEqualToMax) {
		// Assert:
		AssertSizeValidationResult(ValidationResult::Success, 123, 123);
	}

	TEST(TEST_CLASS, FailureWhenValidatingMosaicWithNameSizeGreaterThanMax) {
		// Assert:
		AssertSizeValidationResult(Failure_Mosaic_Invalid_Name, 124, 123);
		AssertSizeValidationResult(Failure_Mosaic_Invalid_Name, 200, 123);
	}

	TEST(TEST_CLASS, FailureWhenValidatingEmptyMosaicName) {
		// Assert:
		AssertSizeValidationResult(Failure_Mosaic_Invalid_Name, 0, 123);
	}

	// endregion

	// region name characters

	namespace {
		void AssertNameValidationResult(ValidationResult expectedResult, const std::string& name) {
			// Arrange:
			auto pValidator = CreateMosaicNameValidator(static_cast<uint8_t>(name.size()));
			auto notification = CreateMosaicNameNotification(
					static_cast<uint8_t>(name.size()),
					reinterpret_cast<const uint8_t*>(name.data()));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "mosaic with name" << name;
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingValidMosaicNames) {
		// Assert:
		for (const auto& name : { "a", "be", "cat", "doom", "al-ce", "al_ce", "alice-", "alice_" })
			AssertNameValidationResult(ValidationResult::Success, name);
	}

	TEST(TEST_CLASS, FailureWhenValidatingInvalidMosaicNames) {
		// Assert:
		for (const auto& name : { "-alice", "_alice", "al.ce", "alIce", "al ce", "al@ce", "al#ce", "!@#$%" })
			AssertNameValidationResult(Failure_Mosaic_Invalid_Name, name);
	}

	// endregion

	// region name and id consistency

	TEST(TEST_CLASS, SuccessWhenValidatingMosaicWithMatchingNameAndId) {
		// Arrange: note that CreateMosaicNameNotification creates proper id
		auto pValidator = CreateMosaicNameValidator(100);
		auto name = std::string(10, 'a');
		auto notification = CreateMosaicNameNotification(static_cast<uint8_t>(name.size()), reinterpret_cast<const uint8_t*>(name.data()));

		// Act:
		auto result = test::ValidateNotification(*pValidator, notification);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);
	}

	TEST(TEST_CLASS, FailureWhenValidatingMosaicWithMismatchedNameAndId) {
		// Arrange: corrupt the id
		auto pValidator = CreateMosaicNameValidator(100);
		auto name = std::string(10, 'a');
		auto notification = CreateMosaicNameNotification(static_cast<uint8_t>(name.size()), reinterpret_cast<const uint8_t*>(name.data()));
		notification.MosaicId = notification.MosaicId + MosaicId(1);

		// Act:
		auto result = test::ValidateNotification(*pValidator, notification);

		// Assert:
		EXPECT_EQ(Failure_Mosaic_Name_Id_Mismatch, result);
	}

	// endregion
}}
