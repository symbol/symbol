/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "catapult/validators/ValidationResult.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS ValidationResultTests

	namespace {
		constexpr ValidationResult MakeValidationResult(ResultSeverity severity, uint8_t facility, uint16_t code, ResultFlags flags) {
			return MakeValidationResult(severity, static_cast<FacilityCode>(facility), code, flags);
		}

		constexpr ValidationResult MakeValidationResult(uint8_t severity, uint8_t facility, uint16_t code, uint8_t flags) {
			return MakeValidationResult(static_cast<ResultSeverity>(severity), facility, code, static_cast<ResultFlags>(flags));
		}
	}

	// region MakeValidationResult / DEFINE_VALIDATION_RESULT

	TEST(TEST_CLASS, CanMakeValidationResult) {
		// Assert: zeros
		EXPECT_EQ(static_cast<ValidationResult>(0x00000000), MakeValidationResult(0, 0, 0, 0));

		// - max single component value
		EXPECT_EQ(static_cast<ValidationResult>(0xC0000000), MakeValidationResult(0xFF, 0, 0, 0));
		EXPECT_EQ(static_cast<ValidationResult>(0x00FF0000), MakeValidationResult(0, 0xFF, 0, 0));
		EXPECT_EQ(static_cast<ValidationResult>(0x0000FFFF), MakeValidationResult(0, 0, 0xFFFF, 0));
		EXPECT_EQ(static_cast<ValidationResult>(0x3F000000), MakeValidationResult(0, 0, 0, 0xFF));

		// - all component values
		EXPECT_EQ(static_cast<ValidationResult>(0x47020005), MakeValidationResult(1, 2, 5, 7));
	}

	TEST(TEST_CLASS, CanMakeValidationResultViaMacro) {
		// Act
		DEFINE_VALIDATION_RESULT(Neutral, Core, Alpha, 0x1234, Verbose);
		DEFINE_VALIDATION_RESULT(Success, Chain, Beta, 0x8800, None);
		DEFINE_VALIDATION_RESULT(Failure, Transfer, Gamma, 0x00AB, None);

		// Assert:
		EXPECT_EQ(static_cast<ValidationResult>(0x41431234), Neutral_Core_Alpha);
		EXPECT_EQ(static_cast<ValidationResult>(0x00FF8800), Success_Chain_Beta);
		EXPECT_EQ(static_cast<ValidationResult>(0x805400AB), Failure_Transfer_Gamma);
	}

	// endregion

	// region GetSeverity / IsSet

	TEST(TEST_CLASS, CanExtractSeverityFromWellKnownValues) {
		// Arrange:
		EXPECT_EQ(ResultSeverity::Success, GetSeverity(ValidationResult::Success));
		EXPECT_EQ(ResultSeverity::Neutral, GetSeverity(ValidationResult::Neutral));
		EXPECT_EQ(ResultSeverity::Failure, GetSeverity(ValidationResult::Failure));
	}

	TEST(TEST_CLASS, CanExtractSeverityFromArbitraryValues) {
		// Arrange:
		for (auto severity : { ResultSeverity::Success, ResultSeverity::Neutral, ResultSeverity::Failure }) {
			// Assert:
			std::string message = "severity " + std::to_string(static_cast<uint32_t>(severity));
			EXPECT_EQ(severity, GetSeverity(MakeValidationResult(severity, 0, 0, ResultFlags::None))) << message;
			EXPECT_EQ(severity, GetSeverity(MakeValidationResult(severity, 7, 9, ResultFlags::Verbose))) << message;
		}
	}

	TEST(TEST_CLASS, CanCheckWhetherOrNotResultFlagIsSet) {
		// Assert: none
		auto result = MakeValidationResult(ResultSeverity::Success, 0, 0, static_cast<ResultFlags>(0));
		EXPECT_FALSE(IsSet(result, static_cast<ResultFlags>(0x01)));
		EXPECT_FALSE(IsSet(result, static_cast<ResultFlags>(0x20)));
		EXPECT_FALSE(IsSet(result, static_cast<ResultFlags>(0x21)));

		// - all
		result = MakeValidationResult(ResultSeverity::Success, 0, 0, static_cast<ResultFlags>(0xFF));
		EXPECT_TRUE(IsSet(result, static_cast<ResultFlags>(0x01)));
		EXPECT_TRUE(IsSet(result, static_cast<ResultFlags>(0x20)));
		EXPECT_TRUE(IsSet(result, static_cast<ResultFlags>(0x3F)));
		EXPECT_FALSE(IsSet(result, static_cast<ResultFlags>(0x40))); // highest two flag bits are not encoded into result
		EXPECT_FALSE(IsSet(result, static_cast<ResultFlags>(0x41)));

		// - some
		result = MakeValidationResult(ResultSeverity::Success, 0, 0, static_cast<ResultFlags>(0x14));
		EXPECT_TRUE(IsSet(result, static_cast<ResultFlags>(0x10)));
		EXPECT_TRUE(IsSet(result, static_cast<ResultFlags>(0x04)));
		EXPECT_TRUE(IsSet(result, static_cast<ResultFlags>(0x14)));
		EXPECT_FALSE(IsSet(result, static_cast<ResultFlags>(0x07)));
		EXPECT_FALSE(IsSet(result, static_cast<ResultFlags>(0x08)));
		EXPECT_FALSE(IsSet(result, static_cast<ResultFlags>(0x15)));
	}

	// endregion

	// region IsValidationResultSuccess / IsValidationResultFailure

	namespace {
		ValidationResult CreateCustomValidationResult(ResultSeverity severity) {
			return MakeValidationResult(severity, 7, 9, ResultFlags::Verbose);
		}
	}

	TEST(TEST_CLASS, IsValidationResultSuccessReturnsTrueOnlyInCaseOfSuccess) {
		EXPECT_TRUE(IsValidationResultSuccess(ValidationResult::Success));
		EXPECT_TRUE(IsValidationResultSuccess(CreateCustomValidationResult(ResultSeverity::Success)));

		EXPECT_FALSE(IsValidationResultSuccess(ValidationResult::Neutral));
		EXPECT_FALSE(IsValidationResultSuccess(CreateCustomValidationResult(ResultSeverity::Neutral)));

		EXPECT_FALSE(IsValidationResultSuccess(ValidationResult::Failure));
		EXPECT_FALSE(IsValidationResultSuccess(CreateCustomValidationResult(ResultSeverity::Failure)));
	}

	TEST(TEST_CLASS, IsValidationResultFailureReturnsTrueOnlyInCaseOfFailure) {
		EXPECT_FALSE(IsValidationResultFailure(ValidationResult::Success));
		EXPECT_FALSE(IsValidationResultFailure(CreateCustomValidationResult(ResultSeverity::Success)));

		EXPECT_FALSE(IsValidationResultFailure(ValidationResult::Neutral));
		EXPECT_FALSE(IsValidationResultFailure(CreateCustomValidationResult(ResultSeverity::Neutral)));

		EXPECT_TRUE(IsValidationResultFailure(ValidationResult::Failure));
		EXPECT_TRUE(IsValidationResultFailure(CreateCustomValidationResult(ResultSeverity::Failure)));
	}

	// endregion

	// region MapToLogLevel

	TEST(TEST_CLASS, MapToLogLevelReturnsCorrectLevelBasedOnVerbosity) {
		EXPECT_EQ(utils::LogLevel::trace, MapToLogLevel(MakeValidationResult(ResultSeverity::Success, 0, 0, ResultFlags::Verbose)));
		EXPECT_EQ(utils::LogLevel::warning, MapToLogLevel(MakeValidationResult(ResultSeverity::Success, 0, 0, ResultFlags::None)));
	}

	// endregion

	// region insertion operator

	TEST(TEST_CLASS, CanOutputWellKnownEnumValues) {
		EXPECT_EQ("Success", test::ToString(ValidationResult::Success));
		EXPECT_EQ("Neutral", test::ToString(ValidationResult::Neutral));
		EXPECT_EQ("Failure", test::ToString(ValidationResult::Failure));
	}

	TEST(TEST_CLASS, CanOutputKnownPluginEnumValues) {
		// Assert: ordered by facility code name
		EXPECT_EQ("Failure_Aggregate_Too_Many_Cosignatures", test::ToString(static_cast<ValidationResult>(0x80410003)));
		EXPECT_EQ("Failure_Chain_Unlinked", test::ToString(static_cast<ValidationResult>(0x80FF0001)));
		EXPECT_EQ("Failure_Consumer_Remote_Chain_Improper_Link", test::ToString(static_cast<ValidationResult>(0x80FE0005)));
		EXPECT_EQ("Failure_Core_Insufficient_Balance", test::ToString(static_cast<ValidationResult>(0x80430003)));
		EXPECT_EQ("Failure_Extension_Partial_Transaction_Cache_Prune", test::ToString(static_cast<ValidationResult>(0x80450001)));
		EXPECT_EQ("Failure_Hash_Already_Exists", test::ToString(static_cast<ValidationResult>(0x81490001)));
		EXPECT_EQ("Failure_LockHash_Invalid_Mosaic_Amount", test::ToString(static_cast<ValidationResult>(0x80480002)));
		EXPECT_EQ("Failure_LockSecret_Invalid_Hash_Algorithm", test::ToString(static_cast<ValidationResult>(0x80520001)));
		EXPECT_EQ("Failure_Metadata_Value_Too_Large", test::ToString(static_cast<ValidationResult>(0x80440002)));
		EXPECT_EQ("Failure_Mosaic_Invalid_Name", test::ToString(static_cast<ValidationResult>(0x804D0002)));
		EXPECT_EQ("Failure_Multisig_Redundant_Modification", test::ToString(static_cast<ValidationResult>(0x80550003)));
		EXPECT_EQ("Failure_Namespace_Invalid_Name", test::ToString(static_cast<ValidationResult>(0x804E0002)));
		EXPECT_EQ("Failure_RestrictionAccount_Invalid_Modification_Address", test::ToString(static_cast<ValidationResult>(0x80500003)));
		EXPECT_EQ("Failure_RestrictionMosaic_Max_Restrictions_Exceeded", test::ToString(static_cast<ValidationResult>(0x80510004)));
		EXPECT_EQ("Failure_Signature_Not_Verifiable", test::ToString(static_cast<ValidationResult>(0x80530001)));
		EXPECT_EQ("Failure_Transfer_Message_Too_Large", test::ToString(static_cast<ValidationResult>(0x80540001)));
	}

	TEST(TEST_CLASS, CanOutputUnknownEnumValues) {
		EXPECT_EQ("ValidationResult<0xABCD9812>", test::ToString(static_cast<ValidationResult>(0xABCD9812)));
		EXPECT_EQ("ValidationResult<0x00CD9812>", test::ToString(static_cast<ValidationResult>(0x00CD9812)));
	}

	// endregion
}}
