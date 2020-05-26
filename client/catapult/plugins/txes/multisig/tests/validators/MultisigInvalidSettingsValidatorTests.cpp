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
#include "src/cache/MultisigCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/MultisigCacheTestUtils.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MultisigInvalidSettingsValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MultisigInvalidSettings,)

	namespace {
		auto CreateNotification(const Address& multisig, int8_t minRemovalDelta, int8_t minApprovalDelta) {
			return model::MultisigSettingsNotification(multisig, minRemovalDelta, minApprovalDelta);
		}

		auto GetValidationResult(const cache::CatapultCache& cache, const model::MultisigSettingsNotification& notification) {
			// Arrange:
			auto pValidator = CreateMultisigInvalidSettingsValidator();

			// Act:
			return test::ValidateNotification(*pValidator, notification, cache);
		}
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsUnknownAndDeltasAreSetToMinusOne) {
		// Arrange:
		auto multisig = test::GenerateRandomByteArray<Address>();
		auto notification = CreateNotification(multisig, -1, -1);

		auto cache = test::MultisigCacheFactory::Create();

		// Act:
		auto result = GetValidationResult(cache, notification);

		// Assert: for explanation of the behavior see comment in validator
		EXPECT_EQ(ValidationResult::Success, result);
	}

	TEST(TEST_CLASS, FailureWhenAccountIsUnknownAndAtLeastOneDeltaIsNotSetToMinusOne) {
		// Arrange:
		auto multisig = test::GenerateRandomByteArray<Address>();
		std::vector<model::MultisigSettingsNotification> notifications{
			CreateNotification(multisig, 0, 1),
			CreateNotification(multisig, 0, -1),
			CreateNotification(multisig, -1, 0)
		};
		std::vector<ValidationResult> results;

		auto cache = test::MultisigCacheFactory::Create();

		// Act:
		for (const auto& notification : notifications)
			results.push_back(GetValidationResult(cache, notification));

		// Assert:
		auto i = 0u;
		for (auto result : results) {
			EXPECT_EQ(Failure_Multisig_Min_Setting_Out_Of_Range, result) << "at index " << i;
			++i;
		}
	}

	// region basic bound check

	namespace {
		struct MultisigSettings {
		public:
			uint8_t Current;
			int8_t Delta;
		};

		std::ostream& operator<<(std::ostream& out, MultisigSettings settings) {
			out << "(" << static_cast<int>(settings.Current) << "," << static_cast<int>(settings.Delta) << ")";
			return out;
		}

		void AssertTestWithSettings(
				ValidationResult expectedResult,
				size_t numCosignatories,
				MultisigSettings removal,
				MultisigSettings approval) {
			// Arrange:
			auto addresses = test::GenerateRandomDataVector<Address>(1 + numCosignatories);
			const auto& multisig = addresses[0];
			auto notification = CreateNotification(multisig, removal.Delta, approval.Delta);

			auto cache = test::MultisigCacheFactory::Create();
			{
				auto delta = cache.createDelta();

				// - create multisig entry in cache
				auto& multisigDelta = delta.sub<cache::MultisigCache>();
				multisigDelta.insert(state::MultisigEntry(multisig));
				auto& entry = multisigDelta.find(multisig).get();
				entry.setMinRemoval(removal.Current);
				entry.setMinApproval(approval.Current);
				auto& cosignatories = entry.cosignatoryAddresses();
				for (auto i = 0u; i < numCosignatories; ++i)
					cosignatories.insert(addresses[1 + i]);

				cache.commit(Height(1));
			}

			// Act:
			auto result = GetValidationResult(cache, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "removal: " << removal << ", approval: " << approval;
		}

		struct ValidTraits {
		public:
			static auto Data() {
				return std::vector<MultisigSettings>{
					{ 1, 1 },
					{ 0, 9 },
					{ 3, 4 },
					{ 2, 0 }
				};
			}
		};

		struct NotPositiveTraits {
		public:
			static auto Data() {
				return std::vector<MultisigSettings>{
					{ 0, 0 },
					{ 0, -1 },
					{ 1, -1 },
					{ 127, -128 },
					{ 0, -128 }
				};
			}
		};

		struct EqualTo15Traits {
		public:
			static auto Data() {
				return std::vector<MultisigSettings>{
					{ 0, 15 },
					{ 2, 15 - 2 },
					{ 15, 0 },
					{ 20, -5 }
				};
			}
		};

		struct GreaterThan15Traits {
		public:
			static auto Data() {
				return std::vector<MultisigSettings>{
					{ 0, 16 },
					{ 2, 16 - 2 },
					{ 16, 0 },
					{ 20, -1 },
					{ 20, -4 }
				};
			}
		};

		template<typename TContainer>
		auto Get(const TContainer& container, size_t index) {
			return container[index % container.size()];
		}

		template<typename TRemovalTraits, typename TApprovalTraits>
		void RunTest(ValidationResult expectedResult, size_t numCosignatories) {
			auto removal = TRemovalTraits::Data();
			auto approval = TApprovalTraits::Data();
			auto count = std::max(removal.size(), approval.size());

			for (auto i = 0u; i < count; ++i)
				AssertTestWithSettings(expectedResult, numCosignatories, Get(removal, i), Get(approval, count - i - 1));
		}
	}

#define TRAITS_BASED_SETTINGS_TEST(TEST_NAME, TRAITS_NAME) \
	template<typename TRemovalTraits, typename TApprovalTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_RemovalInvalid_ApprovalValid) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TRAITS_NAME, ValidTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_RemovalValid_ApprovalInvalid) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ValidTraits, TRAITS_NAME>(); } \
	TEST(TEST_CLASS, TEST_NAME##_BothInvalid) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TRAITS_NAME, TRAITS_NAME>(); } \
	template<typename TRemovalTraits, typename TApprovalTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TEST(TEST_CLASS, SuccessWhenBothResultingSettingsAreWithinBounds) {
		RunTest<ValidTraits, ValidTraits>(ValidationResult::Success, 10);
	}

	TRAITS_BASED_SETTINGS_TEST(FailureWhenResultingSettingIsNotPositive, NotPositiveTraits) {
		RunTest<TRemovalTraits, TApprovalTraits>(Failure_Multisig_Min_Setting_Out_Of_Range, 10);
	}

	TRAITS_BASED_SETTINGS_TEST(SuccessWhenResultingSettingIsLessThanNumberOfCosignatories, GreaterThan15Traits) {
		RunTest<TRemovalTraits, TApprovalTraits>(ValidationResult::Success, 400);
	}

	TRAITS_BASED_SETTINGS_TEST(SuccessWhenResultingSettingIsEqualToNumberOfCosignatories, EqualTo15Traits) {
		RunTest<TRemovalTraits, TApprovalTraits>(ValidationResult::Success, 15);
	}

	TRAITS_BASED_SETTINGS_TEST(FailureWhenResultingSettingIsGreaterThanNumberOfCosignatories, GreaterThan15Traits) {
		RunTest<TRemovalTraits, TApprovalTraits>(Failure_Multisig_Min_Setting_Larger_Than_Num_Cosignatories, 15);
	}

	// endregion
}}
