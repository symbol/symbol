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

#include "src/validators/KeyLinkValidators.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/utils/HexParser.h"
#include "catapult/validators/ValidatorContext.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS VrfKeyLinkPublicKeyValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(VrfKeyLinkPublicKey, Height())

	namespace {
		constexpr auto Success_Result = ValidationResult::Success;
		constexpr auto Failure_Result = Failure_Core_Invalid_Vrf_Key;
		constexpr auto Fork_Height = Height(1000);

		// a genuine ed25519 public key is canonical, in the main subgroup and not the neutral element
		Key ValidVrfPublicKey() {
			return crypto::KeyPair::FromString("9D61B19DEFFD5A60BA844AF492EC2CC44449C5697B326919703BAC031CAE7F60").publicKey();
		}

		// on the curve and canonical but outside the main (prime-order) subgroup
		Key NonSubgroupPublicKey() {
			return utils::ParseByteArray<Key>("C7176A703D4DD84FBA3C0B760D10670F2A2053FA2C39CCC64EC7FD7792AC037A");
		}

		// the neutral element (point of infinity), encoded as 01 00 ... 00; it is in the main subgroup but unbinds
		// a vrf proof from its challenge
		Key NeutralPublicKey() {
			Key key{};
			key[0] = 0x01;
			return key;
		}

		void AssertValidationResult(
				ValidationResult expectedResult,
				const Key& linkedPublicKey,
				model::LinkAction linkAction,
				Height height) {
			// Arrange:
			auto cache = test::CreateEmptyCatapultCache();
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(height, readOnlyCache);

			auto pValidator = CreateVrfKeyLinkPublicKeyValidator(Fork_Height);
			auto mainAccountPublicKey = test::GenerateRandomByteArray<Key>();
			model::VrfKeyLinkNotification notification(mainAccountPublicKey, linkedPublicKey, linkAction);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	// region at and after fork height

	TEST(TEST_CLASS, SuccessWhenLinkingValidKeyAtForkHeight) {
		AssertValidationResult(Success_Result, ValidVrfPublicKey(), model::LinkAction::Link, Fork_Height);
		AssertValidationResult(Success_Result, ValidVrfPublicKey(), model::LinkAction::Link, Fork_Height + Height(100));
	}

	TEST(TEST_CLASS, FailureWhenLinkingNeutralKeyAtForkHeight) {
		AssertValidationResult(Failure_Result, NeutralPublicKey(), model::LinkAction::Link, Fork_Height);
	}

	TEST(TEST_CLASS, FailureWhenLinkingUnsetKeyAtForkHeight) {
		AssertValidationResult(Failure_Result, Key(), model::LinkAction::Link, Fork_Height);
	}

	TEST(TEST_CLASS, FailureWhenLinkingNonSubgroupKeyAtForkHeight) {
		AssertValidationResult(Failure_Result, NonSubgroupPublicKey(), model::LinkAction::Link, Fork_Height);
	}

	// endregion

	// region unlink is not constrained

	TEST(TEST_CLASS, SuccessWhenUnlinkingInvalidKeyAtForkHeight) {
		// Assert: unlink carries the previously linked key and must not be rejected by the key check
		AssertValidationResult(Success_Result, NeutralPublicKey(), model::LinkAction::Unlink, Fork_Height);
		AssertValidationResult(Success_Result, Key(), model::LinkAction::Unlink, Fork_Height);
		AssertValidationResult(Success_Result, NonSubgroupPublicKey(), model::LinkAction::Unlink, Fork_Height);
	}

	// endregion

	// region before fork height (historical replay)

	TEST(TEST_CLASS, SuccessWhenLinkingInvalidKeyBeforeForkHeight) {
		// Assert: the check is disabled before the fork so historical link transactions replay unchanged
		AssertValidationResult(Success_Result, NeutralPublicKey(), model::LinkAction::Link, Fork_Height - Height(1));
		AssertValidationResult(Success_Result, Key(), model::LinkAction::Link, Fork_Height - Height(1));
		AssertValidationResult(Success_Result, NonSubgroupPublicKey(), model::LinkAction::Link, Height(1));
	}

	TEST(TEST_CLASS, SuccessWhenLinkingValidKeyBeforeForkHeight) {
		AssertValidationResult(Success_Result, ValidVrfPublicKey(), model::LinkAction::Link, Fork_Height - Height(1));
	}

	// endregion
}}
