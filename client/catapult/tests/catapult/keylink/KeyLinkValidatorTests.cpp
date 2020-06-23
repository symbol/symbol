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

#include "catapult/keylink/KeyLinkValidator.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/catapult/keylink/test/KeyLinkTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace keylink {

#define TEST_CLASS KeyLinkValidatorTests

	// region test utils

	namespace {
		using Notification = model::BasicKeyLinkNotification<Key, static_cast<model::NotificationType>(0)>;

		struct Accessor {
			static constexpr auto Failure_Link_Already_Exists = static_cast<validators::ValidationResult>(0x80000000 + 1);
			static constexpr auto Failure_Inconsistent_Unlink_Data = static_cast<validators::ValidationResult>(0x80000000 + 2);

			static const auto& Get(const state::AccountState& accountState) {
				return accountState.SupplementalPublicKeys.linked();
			}
		};

		auto CreateKeyLinkValidator(const std::string& name) {
			return keylink::CreateKeyLinkValidator<Notification, Accessor>(name);
		}

		void AssertValidation(
				validators::ValidationResult expectedResult,
				const Key& cacheLinkedPublicKey,
				const Key& notificationLinkedPublicKey,
				model::LinkAction linkAction) {
			// Arrange:
			Key mainAccountPublicKey;
			auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			{
				auto cacheDelta = cache.createDelta();
				mainAccountPublicKey = test::AddAccountWithLink(cacheDelta, cacheLinkedPublicKey);
				cache.commit(Height());
			}

			auto pValidator = CreateKeyLinkValidator("");
			auto notification = Notification(mainAccountPublicKey, notificationLinkedPublicKey, linkAction);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	// endregion

	// region name

	TEST(TEST_CLASS, ValidatorHasCorrectName) {
		// Act:
		auto pObserver = CreateKeyLinkValidator("Foo");

		// Assert:
		EXPECT_EQ("FooKeyLinkValidator", pObserver->name());
	}

	// endregion

	// region link

	TEST(TEST_CLASS, CanLinkWhenLinkIsUnset) {
		AssertValidation(validators::ValidationResult::Success, Key(), test::GenerateRandomByteArray<Key>(), model::LinkAction::Link);
	}

	TEST(TEST_CLASS, CannotLinkWhenLinkIsSet) {
		AssertValidation(
				Accessor::Failure_Link_Already_Exists,
				test::GenerateRandomByteArray<Key>(),
				test::GenerateRandomByteArray<Key>(),
				model::LinkAction::Link);
	}

	// endregion

	// region unlink

	TEST(TEST_CLASS, CanUnlinkWhenLinkIsSetAndLinkedPublicKeyMatches) {
		auto linkedPublicKey = test::GenerateRandomByteArray<Key>();
		AssertValidation(validators::ValidationResult::Success, linkedPublicKey, linkedPublicKey, model::LinkAction::Unlink);
	}

	TEST(TEST_CLASS, CannotUnlinkWhenLinkIsSetAndLinkedPublicKeyDoesNotMatch) {
		AssertValidation(
				Accessor::Failure_Inconsistent_Unlink_Data,
				test::GenerateRandomByteArray<Key>(),
				test::GenerateRandomByteArray<Key>(),
				model::LinkAction::Unlink);
	}

	TEST(TEST_CLASS, CannotUnlinkWhenUnset) {
		AssertValidation(Accessor::Failure_Inconsistent_Unlink_Data, Key(), Key(), model::LinkAction::Unlink);
	}

	// endregion
}}
