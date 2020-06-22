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

#include "catapult/keylink/MultiKeyLinkValidator.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/catapult/keylink/test/KeyLinkTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace keylink {

#define TEST_CLASS MultiKeyLinkValidatorTests

	// region test utils

	namespace {
		using Notification = model::BasicKeyLinkNotification<model::PinnedVotingKey, static_cast<model::NotificationType>(0)>;

		struct Accessor {
			static constexpr auto Failure_Link_Already_Exists = static_cast<validators::ValidationResult>(0x80000000 + 1);
			static constexpr auto Failure_Inconsistent_Unlink_Data = static_cast<validators::ValidationResult>(0x80000000 + 2);
			static constexpr auto Failure_Too_Many_Links = static_cast<validators::ValidationResult>(0x80000000 + 3);

			static const auto& Get(const state::AccountState& accountState) {
				return accountState.SupplementalPublicKeys.voting();
			}
		};

		auto CreateKeyLinkValidator(const std::string& name) {
			return keylink::CreateMultiKeyLinkValidator<Notification, Accessor>(name, 4);
		}

		void AssertValidation(
				validators::ValidationResult expectedResult,
				const std::vector<model::PinnedVotingKey>& cacheLinkedPublicKeys,
				const model::PinnedVotingKey& notificationLinkedPublicKey,
				model::LinkAction linkAction) {
			// Arrange:
			Key mainAccountPublicKey;
			auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			{
				auto cacheDelta = cache.createDelta();
				mainAccountPublicKey = test::AddAccountWithMultiVotingLinks(cacheDelta, cacheLinkedPublicKeys);
				cache.commit(Height());
			}

			auto pValidator = CreateKeyLinkValidator("");
			auto notification = Notification(mainAccountPublicKey, notificationLinkedPublicKey, linkAction);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		model::PinnedVotingKey CreatePinnedVotingKey(FinalizationPoint::ValueType startPoint, FinalizationPoint::ValueType endPoint) {
			return { test::GenerateRandomByteArray<VotingKey>(), FinalizationPoint(startPoint), FinalizationPoint(endPoint) };
		}

		auto CreatePinnedVotingKeys(size_t count) {
			std::vector<model::PinnedVotingKey> pinnedVotingKeys;
			for (auto i = 0u; i < count; ++i)
				pinnedVotingKeys.push_back(CreatePinnedVotingKey((i + 1) * 100, (i + 1) * 100 + 50));

			return pinnedVotingKeys;
		}
	}

	// endregion

	// region name

	TEST(TEST_CLASS, ValidatorHasCorrectName) {
		// Act:
		auto pObserver = CreateKeyLinkValidator("Foo");

		// Assert:
		EXPECT_EQ("FooMultiKeyLinkValidator", pObserver->name());
	}

	// endregion

	// region link

	TEST(TEST_CLASS, CanLinkWhenLinkIsNotConflicting) {
		// Arrange:
		auto pinnedVotingKeys = CreatePinnedVotingKeys(2);
		auto notificationPinnedVotingKey1 = CreatePinnedVotingKey(800, 900);
		auto notificationPinnedVotingKey2 = CreatePinnedVotingKey(800, 900);
		notificationPinnedVotingKey2.StartPoint = pinnedVotingKeys[1].EndPoint + FinalizationPoint(1);

		// Act + Assert:
		AssertValidation(validators::ValidationResult::Success, pinnedVotingKeys, notificationPinnedVotingKey1, model::LinkAction::Link);
		AssertValidation(validators::ValidationResult::Success, pinnedVotingKeys, notificationPinnedVotingKey2, model::LinkAction::Link);
	}

	TEST(TEST_CLASS, CannotLinkWhenLinkIsConflicting) {
		// Arrange:
		auto pinnedVotingKeys = CreatePinnedVotingKeys(2);
		auto notificationPinnedVotingKey1 = CreatePinnedVotingKey(180, 210);
		auto notificationPinnedVotingKey2 = CreatePinnedVotingKey(800, 900);
		notificationPinnedVotingKey2.StartPoint = pinnedVotingKeys[1].EndPoint;

		// Act + Assert:
		AssertValidation(Accessor::Failure_Link_Already_Exists, pinnedVotingKeys, notificationPinnedVotingKey1, model::LinkAction::Link);
		AssertValidation(Accessor::Failure_Link_Already_Exists, pinnedVotingKeys, notificationPinnedVotingKey2, model::LinkAction::Link);
	}

	TEST(TEST_CLASS, CanLinkMaxLinks) {
		auto pinnedVotingKeys = CreatePinnedVotingKeys(3);
		auto notificationPinnedVotingKey = CreatePinnedVotingKey(800, 900);
		AssertValidation(validators::ValidationResult::Success, pinnedVotingKeys, notificationPinnedVotingKey, model::LinkAction::Link);
	}

	TEST(TEST_CLASS, CanotLinkMoreThanMaxLinks) {
		auto pinnedVotingKeys = CreatePinnedVotingKeys(4);
		auto notificationPinnedVotingKey = CreatePinnedVotingKey(800, 900);
		AssertValidation(Accessor::Failure_Too_Many_Links, pinnedVotingKeys, notificationPinnedVotingKey, model::LinkAction::Link);
	}

	// endregion

	// region unlink

	TEST(TEST_CLASS, CanUnlinkWhenPreviousLinkExactlyMatches) {
		auto pinnedVotingKeys = CreatePinnedVotingKeys(2);
		AssertValidation(validators::ValidationResult::Success, pinnedVotingKeys, pinnedVotingKeys[0], model::LinkAction::Unlink);
		AssertValidation(validators::ValidationResult::Success, pinnedVotingKeys, pinnedVotingKeys[1], model::LinkAction::Unlink);
	}

	TEST(TEST_CLASS, CannotUnlinkWhenLinkIsSetAndLinkedPublicKeyDoesNotMatchExactly) {
		// Arrange: EndPoint is off by one
		auto pinnedVotingKeys = CreatePinnedVotingKeys(2);
		auto notificationPinnedVotingKey = pinnedVotingKeys[0];
		notificationPinnedVotingKey.EndPoint = pinnedVotingKeys[0].EndPoint + FinalizationPoint(1);

		// Act + Assert:
		AssertValidation(
				Accessor::Failure_Inconsistent_Unlink_Data,
				pinnedVotingKeys,
				notificationPinnedVotingKey,
				model::LinkAction::Unlink);
	}

	TEST(TEST_CLASS, CannotUnlinkWhenUnset) {
		auto notificationPinnedVotingKey = CreatePinnedVotingKey(180, 210);
		AssertValidation(Accessor::Failure_Inconsistent_Unlink_Data, {}, notificationPinnedVotingKey, model::LinkAction::Unlink);
	}

	// endregion
}}
