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
#include "catapult/model/Address.h"
#include "tests/test/AccountRestrictionCacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AddressInteractionValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AddressInteraction,)

	namespace {
		constexpr auto Default_Network = model::NetworkIdentifier::Zero;

		using CacheContents = std::unordered_map<Key, std::vector<Key>, utils::ArrayHasher<Key>>;

		// region Address / Key traits

		struct AddressTraits {
			static model::UnresolvedAddressSet ParticipantsByAddress(const std::vector<Key>& keys) {
				model::UnresolvedAddressSet addresses;
				for (const auto& key : keys)
					addresses.insert(test::UnresolveXor(model::PublicKeyToAddress(key, Default_Network)));

				return addresses;
			}

			static utils::KeySet ParticipantsByKey(const std::vector<Key>&) {
				return {};
			}
		};

		struct KeyTraits {
			static model::UnresolvedAddressSet ParticipantsByAddress(const std::vector<Key>&) {
				return {};
			}

			static utils::KeySet ParticipantsByKey(const std::vector<Key>& keys) {
				return utils::KeySet(keys.cbegin(), keys.cend());
			}
		};

		// endregion

		// region Incoming / Outgoing traits

		struct IncomingTraits {
			static constexpr auto Restriction_Type = model::AccountRestrictionType::Address;

			static const std::pair<Key, Key> GetPartnersOrdered(const Key& source, const Key& partner) {
				return { source, partner };
			}
		};

		struct OutgoingTraits {
			static constexpr auto Restriction_Type = model::AccountRestrictionType::Address | model::AccountRestrictionType::Outgoing;

			static const std::pair<Key, Key> GetPartnersOrdered(const Key& source, const Key& partner) {
				return { partner, source };
			}
		};

		// endregion

		// region test utils

		template<typename TOperationTraits>
		void PopulateCache(
				cache::CatapultCache& cache,
				model::AccountRestrictionType restrictionType,
				const CacheContents& cacheContents) {
			auto delta = cache.createDelta();
			auto& restrictionCacheDelta = delta.sub<cache::AccountRestrictionCache>();
			for (const auto& pair : cacheContents) {
				auto sourceAddress = model::PublicKeyToAddress(pair.first, Default_Network);
				restrictionCacheDelta.insert(state::AccountRestrictions(sourceAddress));
				auto& restrictions = restrictionCacheDelta.find(sourceAddress).get();
				auto& restriction = restrictions.restriction(restrictionType);
				for (const auto& value : pair.second)
					TOperationTraits::Add(restriction, state::ToVector(model::PublicKeyToAddress(value, Default_Network)));
			}

			cache.commit(Height(1));
		}

		template<typename TOperationTraits>
		void AssertValidationResult(
				ValidationResult expectedResult,
				const CacheContents& cacheContents,
				model::AccountRestrictionType restrictionType,
				const Key& source,
				const model::UnresolvedAddressSet& participantsByAddress,
				const utils::KeySet& participantsByKey) {
			// Arrange:
			auto cache = test::AccountRestrictionCacheFactory::Create();
			PopulateCache<TOperationTraits>(cache, restrictionType, cacheContents);
			auto pValidator = CreateAddressInteractionValidator();
			auto entityType = static_cast<model::EntityType>(0x4123);
			auto notification = model::AddressInteractionNotification(source, entityType, participantsByAddress, participantsByKey);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		// endregion
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits, typename TDirectionTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address_Incoming) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits, IncomingTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Key_Incoming) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<KeyTraits, IncomingTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Address_Outgoing) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits, OutgoingTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Key_Outgoing) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<KeyTraits, OutgoingTraits>(); } \
	template<typename TTraits, typename TDirectionTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region failure

	TRAITS_BASED_TEST(FailureWhenOneAddressIsKnownAndOtherAddressIsNotContainedInValues_Allow) {
		// Arrange:
		auto sourceKey = test::GenerateRandomByteArray<Key>();
		auto values = test::GenerateRandomDataVector<Key>(3);
		CacheContents cacheContents{ { sourceKey, values } };
		auto pair = TDirectionTraits::GetPartnersOrdered(test::GenerateRandomByteArray<Key>(), sourceKey);

		// Act:
		AssertValidationResult<test::AllowTraits>(
				Failure_RestrictionAccount_Address_Interaction_Not_Allowed,
				cacheContents,
				TDirectionTraits::Restriction_Type,
				pair.first,
				TTraits::ParticipantsByAddress({ pair.second }),
				TTraits::ParticipantsByKey({ pair.second }));
	}

	TRAITS_BASED_TEST(FailureWhenOneAddressIsKnownAndOtherAddressIsNotContainedInValues_Block) {
		// Arrange:
		auto sourceKey = test::GenerateRandomByteArray<Key>();
		auto values = test::GenerateRandomDataVector<Key>(3);
		CacheContents cacheContents{ { sourceKey, values } };
		auto pair = TDirectionTraits::GetPartnersOrdered(values[1], sourceKey);

		// Act:
		AssertValidationResult<test::BlockTraits>(
				Failure_RestrictionAccount_Address_Interaction_Not_Allowed,
				cacheContents,
				TDirectionTraits::Restriction_Type,
				pair.first,
				TTraits::ParticipantsByAddress({ pair.second }),
				TTraits::ParticipantsByKey({ pair.second }));
	}

	// endregion

	// region success

	namespace {
		template<typename TOperationTraits, typename TTraits, typename TDirectionTraits>
		void AssertSuccessWhenAddressIsNotKnown() {
			// Arrange:
			auto sourceKey = test::GenerateRandomByteArray<Key>();
			auto values = test::GenerateRandomDataVector<Key>(3);
			CacheContents cacheContents{ { sourceKey, values } };
			auto pair = TDirectionTraits::GetPartnersOrdered(values[1], test::GenerateRandomByteArray<Key>());

			// Act:
			AssertValidationResult<TOperationTraits>(
					ValidationResult::Success,
					cacheContents,
					TDirectionTraits::Restriction_Type,
					pair.first,
					TTraits::ParticipantsByAddress({ pair.second }),
					TTraits::ParticipantsByKey({ pair.second }));
		}
	}

	TRAITS_BASED_TEST(SuccessWhenAddressIsNotKnown_Allow_Incoming) {
		// note that this situation will not occur during normal operation
		AssertSuccessWhenAddressIsNotKnown<test::AllowTraits, TTraits, TDirectionTraits>();
	}

	TRAITS_BASED_TEST(SuccessWhenAddressIsNotKnown_Block_Incoming) {
		AssertSuccessWhenAddressIsNotKnown<test::BlockTraits, TTraits, TDirectionTraits>();
	}

	namespace {
		template<typename TOperationTraits, typename TTraits, typename TDirectionTraits>
		void AssertSuccessWhenValuesAreEmpty() {
			// Arrange:
			auto sourceKey = test::GenerateRandomByteArray<Key>();
			CacheContents cacheContents{ { sourceKey, {} } };
			auto pair = TDirectionTraits::GetPartnersOrdered(test::GenerateRandomByteArray<Key>(), sourceKey);

			// Act:
			AssertValidationResult<TOperationTraits>(
					ValidationResult::Success,
					cacheContents,
					TDirectionTraits::Restriction_Type,
					pair.first,
					TTraits::ParticipantsByAddress({ pair.second }),
					TTraits::ParticipantsByKey({ pair.second }));
		}
	}

	TRAITS_BASED_TEST(SuccessWhenAddressIsKnownAndValuesAreEmpty_Allow) {
		AssertSuccessWhenValuesAreEmpty<test::AllowTraits, TTraits, TDirectionTraits>();
	}

	TRAITS_BASED_TEST(SuccessWhenAddressIsKnownAndValuesAreEmpty_Block) {
		AssertSuccessWhenValuesAreEmpty<test::BlockTraits, TTraits, TDirectionTraits>();
	}

	TRAITS_BASED_TEST(SuccessWhenSourceEqualsDestination_Allow) {
		// Arrange: source is not an allowed value and values are not empty
		auto sourceKey = test::GenerateRandomByteArray<Key>();
		CacheContents cacheContents{ { sourceKey, { test::GenerateRandomByteArray<Key>() } } };

		// Act: no need for direction traits to retrieve partners because source and destination are equal
		AssertValidationResult<test::AllowTraits>(
				ValidationResult::Success,
				cacheContents,
				TDirectionTraits::Restriction_Type,
				sourceKey,
				TTraits::ParticipantsByAddress({ sourceKey }),
				TTraits::ParticipantsByKey({ sourceKey }));
	}

	TRAITS_BASED_TEST(SuccessWhenSourceEqualsDestination_Block) {
		// Arrange: source is a blocked value
		auto sourceKey = test::GenerateRandomByteArray<Key>();
		CacheContents cacheContents{ { sourceKey, { sourceKey } } };

		// Act: no need for direction traits to retrieve partners because source and destination are equal
		AssertValidationResult<test::BlockTraits>(
				ValidationResult::Success,
				cacheContents,
				TDirectionTraits::Restriction_Type,
				sourceKey,
				TTraits::ParticipantsByAddress({ sourceKey }),
				TTraits::ParticipantsByKey({ sourceKey }));
	}

	TRAITS_BASED_TEST(SuccessWhenAllConditionsAreMet_Allow) {
		// Arrange:
		auto sourceKey = test::GenerateRandomByteArray<Key>();
		auto values = test::GenerateRandomDataVector<Key>(3);
		CacheContents cacheContents{ { sourceKey, values } };
		auto pair = TDirectionTraits::GetPartnersOrdered(values[1], sourceKey);

		// Act: pair.first is known and pair.second is an allowed value
		AssertValidationResult<test::AllowTraits>(
				ValidationResult::Success,
				cacheContents,
				TDirectionTraits::Restriction_Type,
				pair.first,
				TTraits::ParticipantsByAddress({ pair.second }),
				TTraits::ParticipantsByKey({ pair.second }));
	}

	TRAITS_BASED_TEST(SuccessWhenAllConditionsAreMet_Block) {
		// Arrange:
		auto sourceKey = test::GenerateRandomByteArray<Key>();
		auto values = test::GenerateRandomDataVector<Key>(3);
		CacheContents cacheContents{ { sourceKey, values } };
		auto pair = TDirectionTraits::GetPartnersOrdered(test::GenerateRandomByteArray<Key>(), sourceKey);

		// Act: pair.first is known and pair.second is not a blocked value
		AssertValidationResult<test::BlockTraits>(
				ValidationResult::Success,
				cacheContents,
				TDirectionTraits::Restriction_Type,
				pair.first,
				TTraits::ParticipantsByAddress({ pair.second }),
				TTraits::ParticipantsByKey({ pair.second }));
	}

	// endregion
}}
