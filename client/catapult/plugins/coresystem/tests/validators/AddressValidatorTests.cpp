#include "src/validators/Validators.h"
#include "catapult/model/Address.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AddressValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(Address, static_cast<model::NetworkIdentifier>(123))

	namespace {
		constexpr auto Network_Identifier = static_cast<model::NetworkIdentifier>(123);

		void AssertValidationResult(ValidationResult expectedResult, const Address& address) {
			// Arrange:
			model::AccountAddressNotification notification(address);
			auto pValidator = CreateAddressValidator(Network_Identifier);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "address " << utils::HexFormat(address);
		}
	}

	// region validation

	TEST(TEST_CLASS, SuccessWhenAddressIsCompatibleWithNetwork) {
		// Arrange:
		auto address = PublicKeyToAddress(test::GenerateRandomData<Key_Size>(), Network_Identifier);

		// Assert:
		AssertValidationResult(ValidationResult::Success, address);
	}

	TEST(TEST_CLASS, FailureWhenAddressHasInvalidChecksum) {
		// Arrange:
		auto address = PublicKeyToAddress(test::GenerateRandomData<Key_Size>(), Network_Identifier);
		address[Address_Decoded_Size / 2] ^= 0xFF;

		// Assert:
		AssertValidationResult(Failure_Core_Invalid_Address, address);
	}

	TEST(TEST_CLASS, FailureWhenAddressIsIncompatibleWithNetwork) {
		// Arrange:
		auto address = PublicKeyToAddress(test::GenerateRandomData<Key_Size>(), model::NetworkIdentifier::Mijin_Test);

		// Assert:
		AssertValidationResult(Failure_Core_Invalid_Address, address);
	}

	// endregion
}}
