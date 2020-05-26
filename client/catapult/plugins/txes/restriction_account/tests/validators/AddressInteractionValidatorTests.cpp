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
#include "tests/test/AccountRestrictionCacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AddressInteractionValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AddressInteraction,)

	namespace {
		using CacheContents = std::unordered_map<Address, std::vector<Address>, utils::ArrayHasher<Address>>;

		// region Incoming / Outgoing traits

		struct IncomingTraits {
			static constexpr auto Restriction_Flags = model::AccountRestrictionFlags::Address;

			static const std::pair<Address, Address> GetPartnersOrdered(const Address& source, const Address& partner) {
				return { source, partner };
			}
		};

		struct OutgoingTraits {
			static constexpr auto Restriction_Flags = model::AccountRestrictionFlags::Address | model::AccountRestrictionFlags::Outgoing;

			static const std::pair<Address, Address> GetPartnersOrdered(const Address& source, const Address& partner) {
				return { partner, source };
			}
		};

		// endregion

		// region test utils

		model::UnresolvedAddressSet UnresolveXorAll(const std::vector<Address>& addresses) {
			model::UnresolvedAddressSet unresolvedAddresses;
			for (const auto& address : addresses)
				unresolvedAddresses.insert(test::UnresolveXor(address));

			return unresolvedAddresses;
		}

		template<typename TOperationTraits>
		void PopulateCache(
				cache::CatapultCache& cache,
				model::AccountRestrictionFlags restrictionFlags,
				const CacheContents& cacheContents) {
			auto delta = cache.createDelta();
			auto& restrictionCacheDelta = delta.sub<cache::AccountRestrictionCache>();
			for (const auto& pair : cacheContents) {
				auto source = pair.first;
				restrictionCacheDelta.insert(state::AccountRestrictions(source));
				auto& restrictions = restrictionCacheDelta.find(source).get();
				auto& restriction = restrictions.restriction(restrictionFlags);
				for (const auto& value : pair.second)
					TOperationTraits::Add(restriction, state::ToVector(value));
			}

			cache.commit(Height(1));
		}

		template<typename TOperationTraits>
		void AssertValidationResult(
				ValidationResult expectedResult,
				const CacheContents& cacheContents,
				model::AccountRestrictionFlags restrictionFlags,
				const Address& source,
				const model::UnresolvedAddressSet& participantsByAddress) {
			// Arrange:
			auto cache = test::AccountRestrictionCacheFactory::Create();
			PopulateCache<TOperationTraits>(cache, restrictionFlags, cacheContents);
			auto pValidator = CreateAddressInteractionValidator();
			auto entityType = static_cast<model::EntityType>(0x4123);
			auto notification = model::AddressInteractionNotification(source, entityType, participantsByAddress);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		// endregion
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TDirectionTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address_Incoming) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<IncomingTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Address_Outgoing) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<OutgoingTraits>(); } \
	template<typename TDirectionTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region failure

	TRAITS_BASED_TEST(FailureWhenOneAddressIsKnownAndOtherAddressIsNotContainedInValues_Allow) {
		// Arrange:
		auto source = test::GenerateRandomByteArray<Address>();
		auto values = test::GenerateRandomDataVector<Address>(3);
		CacheContents cacheContents{ { source, values } };
		auto pair = TDirectionTraits::GetPartnersOrdered(test::GenerateRandomByteArray<Address>(), source);

		// Act:
		AssertValidationResult<test::AllowTraits>(
				Failure_RestrictionAccount_Address_Interaction_Prohibited,
				cacheContents,
				TDirectionTraits::Restriction_Flags,
				pair.first,
				UnresolveXorAll({ pair.second }));
	}

	TRAITS_BASED_TEST(FailureWhenOneAddressIsKnownAndOtherAddressIsNotContainedInValues_Block) {
		// Arrange:
		auto source = test::GenerateRandomByteArray<Address>();
		auto values = test::GenerateRandomDataVector<Address>(3);
		CacheContents cacheContents{ { source, values } };
		auto pair = TDirectionTraits::GetPartnersOrdered(values[1], source);

		// Act:
		AssertValidationResult<test::BlockTraits>(
				Failure_RestrictionAccount_Address_Interaction_Prohibited,
				cacheContents,
				TDirectionTraits::Restriction_Flags,
				pair.first,
				UnresolveXorAll({ pair.second }));
	}

	// endregion

	// region success

	namespace {
		template<typename TOperationTraits, typename TDirectionTraits>
		void AssertSuccessWhenAddressIsNotKnown() {
			// Arrange:
			auto source = test::GenerateRandomByteArray<Address>();
			auto values = test::GenerateRandomDataVector<Address>(3);
			CacheContents cacheContents{ { source, values } };
			auto pair = TDirectionTraits::GetPartnersOrdered(values[1], test::GenerateRandomByteArray<Address>());

			// Act:
			AssertValidationResult<TOperationTraits>(
					ValidationResult::Success,
					cacheContents,
					TDirectionTraits::Restriction_Flags,
					pair.first,
					UnresolveXorAll({ pair.second }));
		}
	}

	TRAITS_BASED_TEST(SuccessWhenAddressIsNotKnown_Allow_Incoming) {
		// note that this situation will not occur during normal operation
		AssertSuccessWhenAddressIsNotKnown<test::AllowTraits, TDirectionTraits>();
	}

	TRAITS_BASED_TEST(SuccessWhenAddressIsNotKnown_Block_Incoming) {
		AssertSuccessWhenAddressIsNotKnown<test::BlockTraits, TDirectionTraits>();
	}

	namespace {
		template<typename TOperationTraits, typename TDirectionTraits>
		void AssertSuccessWhenValuesAreEmpty() {
			// Arrange:
			auto source = test::GenerateRandomByteArray<Address>();
			CacheContents cacheContents{ { source, {} } };
			auto pair = TDirectionTraits::GetPartnersOrdered(test::GenerateRandomByteArray<Address>(), source);

			// Act:
			AssertValidationResult<TOperationTraits>(
					ValidationResult::Success,
					cacheContents,
					TDirectionTraits::Restriction_Flags,
					pair.first,
					UnresolveXorAll({ pair.second }));
		}
	}

	TRAITS_BASED_TEST(SuccessWhenAddressIsKnownAndValuesAreEmpty_Allow) {
		AssertSuccessWhenValuesAreEmpty<test::AllowTraits, TDirectionTraits>();
	}

	TRAITS_BASED_TEST(SuccessWhenAddressIsKnownAndValuesAreEmpty_Block) {
		AssertSuccessWhenValuesAreEmpty<test::BlockTraits, TDirectionTraits>();
	}

	TRAITS_BASED_TEST(SuccessWhenSourceEqualsDestination_Allow) {
		// Arrange: source is not an allowed value and values are not empty
		auto source = test::GenerateRandomByteArray<Address>();
		CacheContents cacheContents{ { source, { test::GenerateRandomByteArray<Address>() } } };

		// Act: no need for direction traits to retrieve partners because source and destination are equal
		AssertValidationResult<test::AllowTraits>(
				ValidationResult::Success,
				cacheContents,
				TDirectionTraits::Restriction_Flags,
				source,
				UnresolveXorAll({ source }));
	}

	TRAITS_BASED_TEST(SuccessWhenSourceEqualsDestination_Block) {
		// Arrange: source is a blocked value
		auto source = test::GenerateRandomByteArray<Address>();
		CacheContents cacheContents{ { source, { source } } };

		// Act: no need for direction traits to retrieve partners because source and destination are equal
		AssertValidationResult<test::BlockTraits>(
				ValidationResult::Success,
				cacheContents,
				TDirectionTraits::Restriction_Flags,
				source,
				UnresolveXorAll({ source }));
	}

	TRAITS_BASED_TEST(SuccessWhenAllConditionsAreMet_Allow) {
		// Arrange:
		auto source = test::GenerateRandomByteArray<Address>();
		auto values = test::GenerateRandomDataVector<Address>(3);
		CacheContents cacheContents{ { source, values } };
		auto pair = TDirectionTraits::GetPartnersOrdered(values[1], source);

		// Act: pair.first is known and pair.second is an allowed value
		AssertValidationResult<test::AllowTraits>(
				ValidationResult::Success,
				cacheContents,
				TDirectionTraits::Restriction_Flags,
				pair.first,
				UnresolveXorAll({ pair.second }));
	}

	TRAITS_BASED_TEST(SuccessWhenAllConditionsAreMet_Block) {
		// Arrange:
		auto source = test::GenerateRandomByteArray<Address>();
		auto values = test::GenerateRandomDataVector<Address>(3);
		CacheContents cacheContents{ { source, values } };
		auto pair = TDirectionTraits::GetPartnersOrdered(test::GenerateRandomByteArray<Address>(), source);

		// Act: pair.first is known and pair.second is not a blocked value
		AssertValidationResult<test::BlockTraits>(
				ValidationResult::Success,
				cacheContents,
				TDirectionTraits::Restriction_Flags,
				pair.first,
				UnresolveXorAll({ pair.second }));
	}

	// endregion
}}
