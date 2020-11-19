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

#include "src/cache/MosaicRestrictionCacheUtils.h"
#include "src/cache/MosaicRestrictionCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS MosaicRestrictionCacheUtilsTests

	// region GetMosaicGlobalRestrictionResolvedRules

	namespace {
		void AssertResolvedRules(
				const std::vector<MosaicRestrictionResolvedRule>& expected,
				const std::vector<MosaicRestrictionResolvedRule>& actual) {
			// Assert:
			ASSERT_EQ(expected.size(), actual.size());

			for (auto i = 0u; i < expected.size(); ++i) {
				auto message = "rule at " + std::to_string(i);
				EXPECT_EQ(expected[i].MosaicId, actual[i].MosaicId) << message;
				EXPECT_EQ(expected[i].RestrictionKey, actual[i].RestrictionKey) << message;
				EXPECT_EQ(expected[i].RestrictionValue, actual[i].RestrictionValue) << message;
				EXPECT_EQ(expected[i].RestrictionType, actual[i].RestrictionType) << message;
			}
		}

		void SeedCacheWithMosaicGlobalRestrictionRules(
				MosaicRestrictionCacheDelta& delta,
				uint64_t referenceKey,
				MosaicId referenceMosaicId,
				MosaicId secondReferenceMosaicId) {
			auto restriction1 = state::MosaicGlobalRestriction(MosaicId(222));
			restriction1.set(200, { secondReferenceMosaicId, 111, model::MosaicRestrictionType::NE });
			delta.insert(state::MosaicRestrictionEntry(restriction1));

			auto restriction2 = state::MosaicGlobalRestriction(MosaicId(111));
			restriction2.set(100, { MosaicId(), 999, model::MosaicRestrictionType::LT });
			restriction2.set(referenceKey, { referenceMosaicId, 888, model::MosaicRestrictionType::EQ });
			restriction2.set(300, { MosaicId(), 777, model::MosaicRestrictionType::GE });
			delta.insert(state::MosaicRestrictionEntry(restriction2));
		}
	}

	TEST(TEST_CLASS, GetMosaicGlobalRestrictionResolvedRules_ResolutionSkippedWhenNoRulesAreSet) {
		// Arrange:
		MosaicRestrictionCache cache(CacheConfiguration(), static_cast<model::NetworkIdentifier>(12));
		auto delta = cache.createDelta();
		SeedCacheWithMosaicGlobalRestrictionRules(*delta, 200, MosaicId(), MosaicId());

		// Act:
		std::vector<MosaicRestrictionResolvedRule> resolvedRules;
		auto result = GetMosaicGlobalRestrictionResolvedRules(delta->asReadOnly(), MosaicId(123), resolvedRules);

		// Assert:
		EXPECT_EQ(MosaicGlobalRestrictionRuleResolutionResult::No_Rules, result);
	}

	namespace {
		void AssertCanResolveMosaicGlobalRestrictionRule(
				uint64_t referenceKey,
				MosaicId referenceMosaicId,
				MosaicId secondReferenceMosaicId,
				const MosaicRestrictionResolvedRule& expectedSecondResolvedRule) {
			// Arrange:
			MosaicRestrictionCache cache(CacheConfiguration(), static_cast<model::NetworkIdentifier>(12));
			auto delta = cache.createDelta();
			SeedCacheWithMosaicGlobalRestrictionRules(*delta, referenceKey, referenceMosaicId, secondReferenceMosaicId);

			// Act:
			std::vector<MosaicRestrictionResolvedRule> resolvedRules;
			auto result = GetMosaicGlobalRestrictionResolvedRules(delta->asReadOnly(), MosaicId(111), resolvedRules);

			// Assert:
			EXPECT_EQ(MosaicGlobalRestrictionRuleResolutionResult::Success, result);

			auto expectedResolvedRules = std::vector<MosaicRestrictionResolvedRule>{
				{ MosaicId(111), 100, 999, model::MosaicRestrictionType::LT },
				expectedSecondResolvedRule,
				{ MosaicId(111), 300, 777, model::MosaicRestrictionType::GE }
			};
			AssertResolvedRules(expectedResolvedRules, resolvedRules);
		}
	}

	TEST(TEST_CLASS, GetMosaicGlobalRestrictionResolvedRules_CanResolveRuleWithImplicitSelfReference) {
		// Assert: mosaic restriction entry 111 is in the cache
		AssertCanResolveMosaicGlobalRestrictionRule(200, MosaicId(), MosaicId(), {
			MosaicId(111), 200, 888, model::MosaicRestrictionType::EQ
		});
	}

	TEST(TEST_CLASS, GetMosaicGlobalRestrictionResolvedRules_CanResolveRuleWithSingleLevelReference) {
		// Assert: mosaic restriction entry 222 is in the cache
		AssertCanResolveMosaicGlobalRestrictionRule(200, MosaicId(222), MosaicId(), {
			MosaicId(222), 200, 888, model::MosaicRestrictionType::EQ
		});
	}

	TEST(TEST_CLASS, GetMosaicGlobalRestrictionResolvedRules_CanResolveRuleReferencingNonexistentEntry) {
		// Assert: mosaic restriction entry 333 is not in the cache
		AssertCanResolveMosaicGlobalRestrictionRule(200, MosaicId(333), MosaicId(), {
			MosaicId(333), 200, 888, model::MosaicRestrictionType::EQ
		});
	}

	TEST(TEST_CLASS, GetMosaicGlobalRestrictionResolvedRules_CanResolveRuleReferencingEntryWithNonexistentKey) {
		// Assert: mosaic restriction entry 222 is in the cache but it doesn't have a rule with key 201
		AssertCanResolveMosaicGlobalRestrictionRule(201, MosaicId(222), MosaicId(), {
			MosaicId(222), 201, 888, model::MosaicRestrictionType::EQ
		});
	}

	TEST(TEST_CLASS, GetMosaicGlobalRestrictionResolvedRules_CanResolveRuleWithMultiLevelReference) {
		// Assert: mosaic restriction entry reference loop is ignored supported
		AssertCanResolveMosaicGlobalRestrictionRule(200, MosaicId(222), MosaicId(111), {
			MosaicId(222), 200, 888, model::MosaicRestrictionType::EQ
		});
	}

	TEST(TEST_CLASS, GetMosaicGlobalRestrictionResolvedRules_CannotResolveRuleWithExplicitSelfReference) {
		// Arrange:
		MosaicRestrictionCache cache(CacheConfiguration(), static_cast<model::NetworkIdentifier>(12));
		auto delta = cache.createDelta();
		SeedCacheWithMosaicGlobalRestrictionRules(*delta, 200, MosaicId(111), MosaicId());

		// Act:
		std::vector<MosaicRestrictionResolvedRule> resolvedRules;
		auto result = GetMosaicGlobalRestrictionResolvedRules(delta->asReadOnly(), MosaicId(111), resolvedRules);

		// Assert: mosaic restriction entry cannot reference itself
		EXPECT_EQ(MosaicGlobalRestrictionRuleResolutionResult::Invalid_Rule, result);
	}

	// endregion

	// region EvaluateMosaicRestrictionResolvedRulesForAddress

	namespace {
		void SeedCacheWithMosaicAddressRestrictionRules(MosaicRestrictionCacheDelta& delta, const Address address) {
			auto restriction1 = state::MosaicAddressRestriction(MosaicId(222), address);
			restriction1.set(200, 111);
			delta.insert(state::MosaicRestrictionEntry(restriction1));

			auto restriction2 = state::MosaicAddressRestriction(MosaicId(111), address);
			restriction2.set(100, 998);
			restriction2.set(200, 888);
			restriction2.set(300, 778);
			delta.insert(state::MosaicRestrictionEntry(restriction2));
		}

		void AssertMosaicRestrictionRulesForAddressEvaluation(
				bool expectedResult,
				const std::vector<MosaicRestrictionResolvedRule>& resolvedRules) {
			// Arrange:
			auto address = test::GenerateRandomByteArray<Address>();
			MosaicRestrictionCache cache(CacheConfiguration(), static_cast<model::NetworkIdentifier>(12));
			auto delta = cache.createDelta();
			SeedCacheWithMosaicAddressRestrictionRules(*delta, address);

			// Act:
			auto result = EvaluateMosaicRestrictionResolvedRulesForAddress(delta->asReadOnly(), address, resolvedRules);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, EvaluateMosaicRestrictionResolvedRulesForAddress_SuccessWhenNoRulesSpecified) {
		AssertMosaicRestrictionRulesForAddressEvaluation(true, {});
	}

	TEST(TEST_CLASS, EvaluateMosaicRestrictionResolvedRulesForAddress_SuccessWhenAllRulesFromSingleEntryPass) {
		AssertMosaicRestrictionRulesForAddressEvaluation(true, {
			{ MosaicId(111), 100, 999, model::MosaicRestrictionType::LT },
			{ MosaicId(111), 200, 888, model::MosaicRestrictionType::EQ },
			{ MosaicId(111), 300, 777, model::MosaicRestrictionType::GE }
		});
	}

	TEST(TEST_CLASS, EvaluateMosaicRestrictionResolvedRulesForAddress_SuccessWhenAllRulesFromMultipleEntriesPass) {
		AssertMosaicRestrictionRulesForAddressEvaluation(true, {
			{ MosaicId(111), 100, 999, model::MosaicRestrictionType::LT },
			{ MosaicId(222), 200, 111, model::MosaicRestrictionType::EQ },
			{ MosaicId(111), 300, 777, model::MosaicRestrictionType::GE }
		});
	}

	TEST(TEST_CLASS, EvaluateMosaicRestrictionResolvedRulesForAddress_FailureWhenAnyRuleFails) {
		AssertMosaicRestrictionRulesForAddressEvaluation(false, {
			{ MosaicId(111), 100, 999, model::MosaicRestrictionType::LT },
			{ MosaicId(111), 200, 887, model::MosaicRestrictionType::EQ },
			{ MosaicId(111), 300, 777, model::MosaicRestrictionType::GE }
		});
	}

	TEST(TEST_CLASS, EvaluateMosaicRestrictionResolvedRulesForAddress_FailureWhenRequiredRuleDoesNotExist) {
		AssertMosaicRestrictionRulesForAddressEvaluation(false, {
			{ MosaicId(111), 100, 999, model::MosaicRestrictionType::LT },
			{ MosaicId(111), 201, 888, model::MosaicRestrictionType::EQ },
			{ MosaicId(111), 300, 777, model::MosaicRestrictionType::GE }
		});
	}

	// endregion
}}
