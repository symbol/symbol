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

#include "src/importance/CalculatorUtils.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/state/AccountActivityBuckets.h"
#include "catapult/state/AccountState.h"
#include "tests/TestHarness.h"

namespace catapult { namespace importance {

#define TEST_CLASS CalculatorUtilsTests

	namespace {
		void Set(
				state::AccountActivityBuckets& buckets,
				model::ImportanceHeight startHeight,
				Amount totalFeesPaid,
				uint32_t beneficiaryCount,
				uint64_t rawScore) {
			buckets.update(startHeight, [totalFeesPaid, beneficiaryCount, rawScore](auto& bucket) {
				bucket.TotalFeesPaid = totalFeesPaid;
				bucket.BeneficiaryCount = beneficiaryCount;
				bucket.RawScore = rawScore;
			});
		}
	}

	// region SummarizeAccountActivity - success

	TEST(TEST_CLASS, CanSummarizeAccountActivityWhenNoBucketsArePresent) {
		// Arrange:
		state::AccountActivityBuckets buckets;

		// Act:
		auto activity = SummarizeAccountActivity(model::ImportanceHeight(220), 20, buckets);

		// Assert:
		EXPECT_EQ(Amount(0), activity.TotalFeesPaid);
		EXPECT_EQ(0u, activity.BeneficiaryCount);
		EXPECT_EQ(Importance(0), activity.PreviousImportance);
	}

	TEST(TEST_CLASS, CanSummarizeAccountActivityWhenSingleBucketIsPresentAtSearchHeight) {
		// Arrange:
		state::AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(220), Amount(11), 12, 10);

		// Act:
		auto activity = SummarizeAccountActivity(model::ImportanceHeight(220), 20, buckets);

		// Assert:
		EXPECT_EQ(Amount(11), activity.TotalFeesPaid);
		EXPECT_EQ(12u, activity.BeneficiaryCount);
		EXPECT_EQ(Importance(0), activity.PreviousImportance);
	}

	TEST(TEST_CLASS, CanSummarizeAccountActivityWhenBucketsAreFullStartingAtSearchHeight) {
		// Arrange:
		state::AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(100), Amount(17), 10, 5);
		Set(buckets, model::ImportanceHeight(120), Amount(16), 20, 3);
		Set(buckets, model::ImportanceHeight(140), Amount(15), 30, 1);
		Set(buckets, model::ImportanceHeight(160), Amount(14), 40, 8);
		Set(buckets, model::ImportanceHeight(180), Amount(13), 50, 6);
		Set(buckets, model::ImportanceHeight(200), Amount(12), 60, 4);
		Set(buckets, model::ImportanceHeight(220), Amount(11), 70, 2);

		// Act:
		auto activity = SummarizeAccountActivity(model::ImportanceHeight(220), 20, buckets);

		// Assert:
		EXPECT_EQ(Amount(65), activity.TotalFeesPaid);
		EXPECT_EQ(250u, activity.BeneficiaryCount);
		EXPECT_EQ(Importance(4), activity.PreviousImportance);
	}

	TEST(TEST_CLASS, CanSummarizeAccountActivityWhenSomeBucketsArePresentStartingAtSearchHeight) {
		// Arrange:
		state::AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(180), Amount(13), 50, 6);
		Set(buckets, model::ImportanceHeight(200), Amount(12), 60, 4);
		Set(buckets, model::ImportanceHeight(220), Amount(11), 70, 2);

		// Act:
		auto activity = SummarizeAccountActivity(model::ImportanceHeight(220), 20, buckets);

		// Assert:
		EXPECT_EQ(Amount(36), activity.TotalFeesPaid);
		EXPECT_EQ(180u, activity.BeneficiaryCount);
		EXPECT_EQ(Importance(4), activity.PreviousImportance);
	}

	TEST(TEST_CLASS, CanSummarizeAccountActivityWhenSomeBucketsArePresentStartingAfterSearchHeight) {
		// Arrange:
		state::AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(160), Amount(13), 50, 6);
		Set(buckets, model::ImportanceHeight(180), Amount(12), 60, 4);
		Set(buckets, model::ImportanceHeight(200), Amount(11), 70, 2);

		// Act:
		auto activity = SummarizeAccountActivity(model::ImportanceHeight(220), 20, buckets);

		// Assert:
		EXPECT_EQ(Amount(36), activity.TotalFeesPaid);
		EXPECT_EQ(180u, activity.BeneficiaryCount);
		EXPECT_EQ(Importance(2), activity.PreviousImportance);
	}

	TEST(TEST_CLASS, CanSummarizeAccountActivityWhenSomeBucketsArePresentStartingAtSearchHeightWithGaps) {
		// Arrange:
		state::AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(100), Amount(17), 10, 5);
		Set(buckets, model::ImportanceHeight(120), Amount(16), 20, 3);
		Set(buckets, model::ImportanceHeight(140), Amount(15), 30, 1);
		Set(buckets, model::ImportanceHeight(220), Amount(11), 70, 2);

		// Act:
		auto activity = SummarizeAccountActivity(model::ImportanceHeight(220), 20, buckets);

		// Assert:
		EXPECT_EQ(Amount(26), activity.TotalFeesPaid);
		EXPECT_EQ(100u, activity.BeneficiaryCount);
		EXPECT_EQ(Importance(0), activity.PreviousImportance);
	}

	TEST(TEST_CLASS, CanSummarizeAccountActivityWhenSomeBucketsArePresentStartingAtSearchHeightExtendingToNemesisHeight) {
		// Arrange:
		state::AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(1), Amount(13), 50, 6);
		Set(buckets, model::ImportanceHeight(20), Amount(12), 60, 4);
		Set(buckets, model::ImportanceHeight(40), Amount(11), 70, 2);

		// Act:
		auto activity = SummarizeAccountActivity(model::ImportanceHeight(40), 20, buckets);

		// Assert:
		EXPECT_EQ(Amount(36), activity.TotalFeesPaid);
		EXPECT_EQ(180u, activity.BeneficiaryCount);
		EXPECT_EQ(Importance(4), activity.PreviousImportance);
	}

	TEST(TEST_CLASS, CanSummarizeAccountActivityWhenSomeBucketsArePresentStartingAtSearchHeightExtendingToNemesisHeightWithGaps) {
		// Arrange:
		state::AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(1), Amount(13), 50, 6);
		Set(buckets, model::ImportanceHeight(40), Amount(11), 70, 2);

		// Act:
		auto activity = SummarizeAccountActivity(model::ImportanceHeight(40), 20, buckets);

		// Assert:
		EXPECT_EQ(Amount(24), activity.TotalFeesPaid);
		EXPECT_EQ(120u, activity.BeneficiaryCount);
		EXPECT_EQ(Importance(0), activity.PreviousImportance);
	}

	TEST(TEST_CLASS, CanSummarizeAccountActivityWhenSearchHeightIsNemesisHeight) {
		// Arrange:
		state::AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(1), Amount(13), 50, 6);

		// Act:
		auto activity = SummarizeAccountActivity(model::ImportanceHeight(1), 20, buckets);

		// Assert:
		EXPECT_EQ(Amount(13), activity.TotalFeesPaid);
		EXPECT_EQ(50u, activity.BeneficiaryCount);
		EXPECT_EQ(Importance(0), activity.PreviousImportance);
	}

	TEST(TEST_CLASS, CanSummarizeAccountActivityWhenPreviousSearchHeightIsNemesisHeight) {
		// Arrange:
		state::AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(1), Amount(13), 50, 6);
		Set(buckets, model::ImportanceHeight(20), Amount(12), 60, 4);

		// Act:
		auto activity = SummarizeAccountActivity(model::ImportanceHeight(20), 20, buckets);

		// Assert:
		EXPECT_EQ(Amount(25), activity.TotalFeesPaid);
		EXPECT_EQ(110u, activity.BeneficiaryCount);
		EXPECT_EQ(Importance(6), activity.PreviousImportance);
	}

	// endregion

	// region SummarizeAccountActivity - failure

	TEST(TEST_CLASS, CannotSummarizeAccountActivityGivenImproperSearchHeight) {
		// Arrange:
		state::AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(200), Amount(12), 60, 4);
		Set(buckets, model::ImportanceHeight(220), Amount(11), 70, 2);

		// Act + Assert:
		EXPECT_THROW(SummarizeAccountActivity(model::ImportanceHeight(0), 20, buckets), catapult_invalid_argument);
		EXPECT_THROW(SummarizeAccountActivity(model::ImportanceHeight(219), 20, buckets), catapult_invalid_argument);
		EXPECT_THROW(SummarizeAccountActivity(model::ImportanceHeight(221), 20, buckets), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotSummarizeAccountActivityWhenAnyBucketHasImproperStartHeight) {
		// Arrange:
		state::AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(120), Amount(16), 20, 3);
		Set(buckets, model::ImportanceHeight(140), Amount(15), 30, 1);
		Set(buckets, model::ImportanceHeight(160), Amount(14), 40, 8);
		Set(buckets, model::ImportanceHeight(181), Amount(13), 50, 6); // invalid importance height will cause processing to stop
		Set(buckets, model::ImportanceHeight(200), Amount(12), 60, 4);
		Set(buckets, model::ImportanceHeight(220), Amount(11), 70, 2);

		// Act + Assert:
		EXPECT_THROW(SummarizeAccountActivity(model::ImportanceHeight(220), 20, buckets), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotSummarizeAccountActivityWhenAnyBucketHasStartHeightGreaterThanSearchHeight) {
		// Arrange:
		state::AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(120), Amount(16), 20, 3);
		Set(buckets, model::ImportanceHeight(140), Amount(15), 30, 1);
		Set(buckets, model::ImportanceHeight(160), Amount(14), 40, 8);
		Set(buckets, model::ImportanceHeight(180), Amount(13), 50, 6);
		Set(buckets, model::ImportanceHeight(200), Amount(12), 60, 4);
		Set(buckets, model::ImportanceHeight(220), Amount(11), 70, 2);

		// Act + Assert:
		EXPECT_THROW(SummarizeAccountActivity(model::ImportanceHeight(180), 20, buckets), catapult_invalid_argument);
	}

	// endregion

	// region FinalizeAccountActivity

	TEST(TEST_CLASS, CanFinalizeAccountActivityWhenNoBucketExists) {
		// Arrange:
		state::AccountActivityBuckets buckets;

		// Act:
		FinalizeAccountActivity(model::ImportanceHeight(220), Importance(123), buckets);
		auto bucket = buckets.get(model::ImportanceHeight(220));

		// Assert:
		EXPECT_EQ(model::ImportanceHeight(220), bucket.StartHeight);
		EXPECT_EQ(Amount(0), bucket.TotalFeesPaid);
		EXPECT_EQ(0u, bucket.BeneficiaryCount);
		EXPECT_EQ(123u, bucket.RawScore);
	}

	TEST(TEST_CLASS, CanFinalizeAccountActivityWhenBucketExists) {
		// Arrange:
		state::AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(220), Amount(13), 50, 6);

		// Act:
		FinalizeAccountActivity(model::ImportanceHeight(220), Importance(123), buckets);
		auto bucket = buckets.get(model::ImportanceHeight(220));

		// Assert:
		EXPECT_EQ(model::ImportanceHeight(220), bucket.StartHeight);
		EXPECT_EQ(Amount(13), bucket.TotalFeesPaid);
		EXPECT_EQ(50u, bucket.BeneficiaryCount);
		EXPECT_EQ(123u, bucket.RawScore);
	}

	// endregion

	// region CalculateImportances

	namespace {
		constexpr MosaicId Harvesting_Mosaic_Id(9876);

		model::BlockChainConfiguration CreateBlockChainConfiguration(uint8_t importanceActivityPercentage) {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.HarvestingMosaicId = Harvesting_Mosaic_Id;
			config.TotalChainImportance = Importance(9'000);
			config.ImportanceActivityPercentage = importanceActivityPercentage;
			config.MinHarvesterBalance = Amount(1'000);
			return config;
		}
	}

	TEST(TEST_CLASS, StakeImportanceIsAsExpected) {
		// Arrange:
		state::AccountState accountState(test::GenerateRandomByteArray<Address>(), Height());
		accountState.Balances.credit(Harvesting_Mosaic_Id, Amount(500));
		AccountSummary accountSummary(AccountActivitySummary(), accountState);
		ImportanceCalculationContext importanceContext;
		importanceContext.ActiveHarvestingMosaics = Amount(1'000);
		auto config = CreateBlockChainConfiguration(25);

		// Act:
		CalculateImportances(accountSummary, importanceContext, config);

		// Assert: 9'000 * (500 / 1'000) * ((100 - 25) / 100) = 3'375
		EXPECT_EQ(Importance(3'375), accountSummary.StakeImportance);
		EXPECT_EQ(Importance(), accountSummary.ActivityImportance);
	}

	TEST(TEST_CLASS, ActivityImportanceIsAsExpected_FeesPaid) {
		// Arrange:
		state::AccountState accountState(test::GenerateRandomByteArray<Address>(), Height());
		accountState.Balances.credit(Harvesting_Mosaic_Id, Amount(500));
		AccountSummary accountSummary(AccountActivitySummary(), accountState);
		accountSummary.ActivitySummary.TotalFeesPaid = Amount(200);
		ImportanceCalculationContext importanceContext;
		importanceContext.ActiveHarvestingMosaics = Amount(1'000);
		importanceContext.TotalFeesPaid = Amount(600);
		auto config = CreateBlockChainConfiguration(25);

		// Act:
		CalculateImportances(accountSummary, importanceContext, config);

		// Assert:    stake importance: 9'000 * (500 / 1'000) * ((100 - 25) / 100) = 3'375
		//         activity importance: 9'000 * (200 / 600) * (1'000 / 500) * (25 / 100) * (8 / 10) = 1'200
		EXPECT_EQ(Importance(3'375), accountSummary.StakeImportance);
		EXPECT_EQ(Importance(1'200), accountSummary.ActivityImportance);
	}

	TEST(TEST_CLASS, ActivityImportanceIsAsExpected_BeneficiaryCount) {
		// Arrange:
		state::AccountState accountState(test::GenerateRandomByteArray<Address>(), Height());
		accountState.Balances.credit(Harvesting_Mosaic_Id, Amount(500));
		AccountSummary accountSummary(AccountActivitySummary(), accountState);
		accountSummary.ActivitySummary.BeneficiaryCount = 200;
		ImportanceCalculationContext importanceContext;
		importanceContext.ActiveHarvestingMosaics = Amount(1'000);
		importanceContext.TotalBeneficiaryCount = 600;
		auto config = CreateBlockChainConfiguration(25);

		// Act:
		CalculateImportances(accountSummary, importanceContext, config);

		// Assert:    stake importance: 9'000 * (500 / 1'000) * ((100 - 25) / 100) = 3'375
		//         activity importance: 9'000 * (200 / 600) * (1'000 / 500) * (25 / 100) * (2 / 10) = 300
		EXPECT_EQ(Importance(3'375), accountSummary.StakeImportance);
		EXPECT_EQ(Importance(300), accountSummary.ActivityImportance);
	}

	namespace {
		void AssertActivityImportance(
				uint8_t activityImportancePercentage,
				Importance expectedStateImportance,
				Importance expectedActivityImportance) {
			// Arrange:
			state::AccountState accountState(test::GenerateRandomByteArray<Address>(), Height());
			accountState.Balances.credit(Harvesting_Mosaic_Id, Amount(500));
			AccountSummary accountSummary(AccountActivitySummary(), accountState);
			accountSummary.ActivitySummary.TotalFeesPaid = Amount(200);
			accountSummary.ActivitySummary.BeneficiaryCount = 100;
			ImportanceCalculationContext importanceContext;
			importanceContext.ActiveHarvestingMosaics = Amount(1'000);
			importanceContext.TotalFeesPaid = Amount(600);
			importanceContext.TotalBeneficiaryCount = 300;
			auto config = CreateBlockChainConfiguration(activityImportancePercentage);

			// Act:
			CalculateImportances(accountSummary, importanceContext, config);

			// Assert:
			EXPECT_EQ(expectedStateImportance, accountSummary.StakeImportance);
			EXPECT_EQ(expectedActivityImportance, accountSummary.ActivityImportance);
		}
	}

	TEST(TEST_CLASS, ActivityImportanceIsAsExpected_FeesPaidAndBeneficiaryCount_ActivityPercentageGreaterThanZero) {
		// Assert:    stake importance: 9'000 * (500 / 1'000) * ((100 - 25) / 100) = 3'375
		//         activity importance: 1'200 + 300 = 1'500
		AssertActivityImportance(25, Importance(3'375), Importance(1'500));
	}

	TEST(TEST_CLASS, ActivityImportanceIsAsExpected_FeesPaidAndBeneficiaryCount_ActivityPercentageEqualToZero) {
		// Assert:    stake importance: 9'000 * (500 / 1'000) * (100 / 100) = 4'500
		//         activity importance: 0 + 0 = 0
		AssertActivityImportance(0, Importance(4'500), Importance());
	}

	// endregion
}}
