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

#include "src/validators/Validators.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "plugins/coresystem/tests/test/KeyLinkTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(VotingKeyLink,)
	DEFINE_COMMON_VALIDATOR_TESTS(VrfKeyLink,)

#define TEST_CLASS KeyLinkValidatorTests // used to generate unique function names in macros
#define VOTING_TEST_CLASS VotingKeyLinkValidatorTests
#define VRF_TEST_CLASS VrfKeyLinkValidatorTests

	// region traits

	namespace {
		struct VotingTraits : test::BasicVotingKeyLinkTestTraits {
			static constexpr auto CreateValidator = CreateVotingKeyLinkValidator;
		};

		struct VrfTraits : test::BasicVrfKeyLinkTestTraits {
			static constexpr auto CreateValidator = CreateVrfKeyLinkValidator;
		};
	}

#define KEY_LINK_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(VOTING_TEST_CLASS, TEST_NAME##_Voting) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VotingTraits>(); } \
	TEST(VRF_TEST_CLASS, TEST_NAME##_Vrf) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VrfTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region test utils

	namespace {
		template<typename TTraits>
		void AssertValidation(
				ValidationResult expectedResult,
				const typename TTraits::KeyType& cacheLinkedPublicKey,
				const typename TTraits::KeyType& notificationLinkedPublicKey,
				model::LinkAction linkAction) {
			// Arrange:
			auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			auto mainAccountPublicKey = test::AddAccountWithLink<TTraits>(cache, cacheLinkedPublicKey);

			auto pValidator = TTraits::CreateValidator();
			auto notification = typename TTraits::NotificationType(mainAccountPublicKey, notificationLinkedPublicKey, linkAction);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	// endregion

	// region link

	KEY_LINK_TEST(CanLinkWhenLinkIsUnset) {
		AssertValidation<TTraits>(
				ValidationResult::Success,
				typename TTraits::KeyType(),
				test::GenerateRandomByteArray<typename TTraits::KeyType>(),
				model::LinkAction::Link);
	}

	KEY_LINK_TEST(CannotLinkWhenLinkIsSet) {
		AssertValidation<TTraits>(
				Failure_Core_Link_Already_Exists,
				test::GenerateRandomByteArray<typename TTraits::KeyType>(),
				test::GenerateRandomByteArray<typename TTraits::KeyType>(),
				model::LinkAction::Link);
	}

	// endregion

	// region unlink

	KEY_LINK_TEST(CanUnlinkWhenLinkIsSetAndLinkedPublicKeyMatches) {
		auto linkedPublicKey = test::GenerateRandomByteArray<typename TTraits::KeyType>();
		AssertValidation<TTraits>(ValidationResult::Success, linkedPublicKey, linkedPublicKey, model::LinkAction::Unlink);
	}

	KEY_LINK_TEST(CannotUnlinkWhenLinkIsSetAndLinkedPublicKeyDoesNotMatch) {
		AssertValidation<TTraits>(
				Failure_Core_Inconsistent_Unlink_Data,
				test::GenerateRandomByteArray<typename TTraits::KeyType>(),
				test::GenerateRandomByteArray<typename TTraits::KeyType>(),
				model::LinkAction::Unlink);
	}

	KEY_LINK_TEST(CannotUnlinkWhenUnset) {
		AssertValidation<TTraits>(
				Failure_Core_Inconsistent_Unlink_Data,
				typename TTraits::KeyType(),
				typename TTraits::KeyType(),
				model::LinkAction::Unlink);
	}

	// endregion
}}
