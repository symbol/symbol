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
#include "src/model/AccountOperationRestrictionTransaction.h"
#include "tests/test/AccountRestrictionCacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AccountOperationRestrictionNoSelfBlockingTests

	DEFINE_COMMON_VALIDATOR_TESTS(AccountOperationRestrictionNoSelfBlocking,)

	namespace {
		using Notification = model::ModifyAccountOperationRestrictionValueNotification;

		constexpr auto Add = model::AccountRestrictionModificationType::Add;
		constexpr auto Del = model::AccountRestrictionModificationType::Del;
		constexpr auto Relevant_Entity_Type = model::AccountOperationRestrictionTransaction::Entity_Type;
		constexpr auto Restriction_Type = model::AccountRestrictionType::TransactionType | model::AccountRestrictionType::Outgoing;
		constexpr auto Failure_Result = Failure_RestrictionAccount_Modification_Not_Allowed;

		struct AccountOperationRestrictionTraits : public test::BaseAccountOperationRestrictionTraits {
			using NotificationType = model::ModifyAccountOperationRestrictionValueNotification;
		};

		auto RandomValue() {
			return static_cast<model::EntityType>(test::RandomByte());
		}

		template<typename TOperationTraits>
		void AddToCache(cache::CatapultCache& cache, const Key& key, const std::vector<model::EntityType>& values) {
			auto delta = cache.createDelta();
			auto& restrictionCacheDelta = delta.sub<cache::AccountRestrictionCache>();
			auto address = model::PublicKeyToAddress(key, model::NetworkIdentifier::Zero);
			auto restrictions = state::AccountRestrictions(address);
			auto& restriction = restrictions.restriction(Restriction_Type);
			for (auto value : values)
				TOperationTraits::Add(restriction, state::ToVector(value));

			restrictionCacheDelta.insert(restrictions);
			cache.commit(Height(1));
		}

		void RunValidator(ValidationResult expectedResult, cache::CatapultCache& cache, const Notification& notification) {
			// Arrange:
			auto pValidator = CreateAccountOperationRestrictionNoSelfBlockingValidator();

			// Act:
			auto result = test::ValidateNotification<Notification>(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		enum class CacheSeed { No, Empty_Restrictions, Random_Value, Relevant_Value};

		template<typename TOperationTraits>
		void AssertValidationResult(
				ValidationResult expectedResult,
				CacheSeed cacheSeed,
				const Key& seedKey,
				const Notification& notification) {
			// Arrange:
			auto cache = test::AccountRestrictionCacheFactory::Create();
			switch (cacheSeed) {
			case CacheSeed::No:
				break;
			case CacheSeed::Empty_Restrictions:
				AddToCache<TOperationTraits>(cache, seedKey, {});
				break;
			case CacheSeed::Random_Value:
				AddToCache<TOperationTraits>(cache, seedKey, { RandomValue() });
				break;
			case CacheSeed::Relevant_Value:
				AddToCache<TOperationTraits>(cache, seedKey, { Relevant_Entity_Type });
				break;
			}

			// Act:
			RunValidator(expectedResult, cache, notification);
		}

		template<typename TOperationTraits>
		auto CreateNotification(const Key& key, const model::AccountRestrictionModification<model::EntityType>& modification) {
			return test::CreateNotification<AccountOperationRestrictionTraits, TOperationTraits>(key, modification);
		}
	}

	// region failure

	TEST(TEST_CLASS, FailureWhenAccountIsUnknown_Allow_Add_NotRelevantType) {
		// Arrange:
		auto seedKey = test::GenerateRandomByteArray<Key>();
		auto notificationKey = test::GenerateRandomByteArray<Key>();
		auto modification = model::AccountRestrictionModification<model::EntityType>{ Add, RandomValue() };
		auto notification = CreateNotification<test::AllowTraits>(notificationKey, modification);

		// Act + Assert:
		AssertValidationResult<test::AllowTraits>(Failure_Result, CacheSeed::No, seedKey, notification);
	}

	TEST(TEST_CLASS, FailureWhenAccountIsUnknown_Block_Add_RelevantType) {
		// Arrange:
		auto seedKey = test::GenerateRandomByteArray<Key>();
		auto notificationKey = test::GenerateRandomByteArray<Key>();
		auto modification = model::AccountRestrictionModification<model::EntityType>{ Add, Relevant_Entity_Type };
		auto notification = CreateNotification<test::BlockTraits>(notificationKey, modification);

		// Act + Assert:
		AssertValidationResult<test::BlockTraits>(Failure_Result, CacheSeed::No, seedKey, notification);
	}

	TEST(TEST_CLASS, FailureWhenAccountIsKnown_Allow_Del_RelevantType) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		auto modification = model::AccountRestrictionModification<model::EntityType>{ Del, Relevant_Entity_Type };
		auto notification = CreateNotification<test::AllowTraits>(key, modification);

		// Act + Assert:
		AssertValidationResult<test::AllowTraits>(Failure_Result, CacheSeed::Empty_Restrictions, key, notification);
	}

	TEST(TEST_CLASS, FailureWhenAccountIsKnown_Allow_Add_NotRelevantType) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		auto modification = model::AccountRestrictionModification<model::EntityType>{ Add, RandomValue() };
		auto notification = CreateNotification<test::AllowTraits>(key, modification);

		// Act + Assert:
		AssertValidationResult<test::AllowTraits>(Failure_Result, CacheSeed::Empty_Restrictions, key, notification);
	}

	TEST(TEST_CLASS, FailureWhenAccountIsKnown_Block_Add_RelevantType) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		auto modification = model::AccountRestrictionModification<model::EntityType>{ Add, Relevant_Entity_Type };
		auto notification = CreateNotification<test::BlockTraits>(key, modification);

		// Act + Assert:
		AssertValidationResult<test::BlockTraits>(Failure_Result, CacheSeed::Random_Value, key, notification);
	}

	// endregion

	// region success

	TEST(TEST_CLASS, SuccessWhenAccountIsUnknown_Allow_Add_RelevantType) {
		// Arrange:
		auto seedKey = test::GenerateRandomByteArray<Key>();
		auto notificationKey = test::GenerateRandomByteArray<Key>();
		auto modification = model::AccountRestrictionModification<model::EntityType>{ Add, Relevant_Entity_Type };
		auto notification = CreateNotification<test::AllowTraits>(notificationKey, modification);

		// Act + Assert:
		AssertValidationResult<test::AllowTraits>(ValidationResult::Success, CacheSeed::No, seedKey, notification);
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsUnknown_Block_Add_NotRelevantType) {
		// Arrange:
		auto seedKey = test::GenerateRandomByteArray<Key>();
		auto notificationKey = test::GenerateRandomByteArray<Key>();
		auto modification = model::AccountRestrictionModification<model::EntityType>{ Add, RandomValue() };
		auto notification = CreateNotification<test::BlockTraits>(notificationKey, modification);

		// Act + Assert:
		AssertValidationResult<test::BlockTraits>(ValidationResult::Success, CacheSeed::No, seedKey, notification);
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsKnown_Allow_Add_RelevantType) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		auto modification = model::AccountRestrictionModification<model::EntityType>{ Add, Relevant_Entity_Type };
		auto notification = CreateNotification<test::AllowTraits>(key, modification);

		// Act + Assert:
		AssertValidationResult<test::AllowTraits>(ValidationResult::Success, CacheSeed::Empty_Restrictions, key, notification);
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsKnown_Allow_Add_NotRelevantType_SeededRelevantType) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		auto modification = model::AccountRestrictionModification<model::EntityType>{ Add, RandomValue() };
		auto notification = CreateNotification<test::AllowTraits>(key, modification);

		// Act + Assert:
		AssertValidationResult<test::AllowTraits>(ValidationResult::Success, CacheSeed::Relevant_Value, key, notification);
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsKnown_Block_Add_NotRelevantType) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		auto modification = model::AccountRestrictionModification<model::EntityType>{ Add, RandomValue() };
		auto notification = CreateNotification<test::BlockTraits>(key, modification);

		// Act + Assert:
		AssertValidationResult<test::BlockTraits>(ValidationResult::Success, CacheSeed::Random_Value, key, notification);
	}

	// endregion
}}
