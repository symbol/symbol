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

#include "src/importance/ActivityObserverUtils.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/test/plugins/AccountObserverTestContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace importance {

#define TEST_CLASS ActivityObserverUtilsTests

	namespace {
		constexpr auto Harvesting_Mosaic_Id = MosaicId(987);
		constexpr auto Notification_Height = Height(100);
		constexpr auto Importance_Height = model::ImportanceHeight(98);

		// region test context

		class TestContext : public test::AccountObserverTestContext {
		public:
			TestContext(observers::NotifyMode notifyMode, Amount minHarvesterBalance)
					: test::AccountObserverTestContext(notifyMode, Notification_Height, CreateBlockChainConfiguration(minHarvesterBalance))
			{}

		public:
			auto addAccount(const Address& address, Amount harvestingBalance) {
				auto& accountStateCache = cache().sub<cache::AccountStateCache>();
				accountStateCache.addAccount(address, Height(123));

				auto accountStateIter = accountStateCache.find(address);
				accountStateIter.get().Balances.credit(Harvesting_Mosaic_Id, harvestingBalance);
				return accountStateIter;
			}

		public:
			void update(const Address& address) {
				auto commitAction = [](auto& bucket) {
					bucket.BeneficiaryCount += 2;
				};
				auto rollbackAction = [](auto& bucket) {
					bucket.BeneficiaryCount -= 2;
				};

				UpdateActivity(address, observerContext(), commitAction, rollbackAction);
			}

		private:
			static model::BlockChainConfiguration CreateBlockChainConfiguration(Amount minHarvesterBalance) {
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.HarvestingMosaicId = Harvesting_Mosaic_Id;
				config.ImportanceGrouping = 2;
				config.MinHarvesterBalance = minHarvesterBalance;
				return config;
			}
		};

		// endregion
	}

	// region eligibility check

	namespace {
		void AssertUpdateActivityBypassesUpdateOfAccountThatCannotHarvest(observers::NotifyMode notifyMode) {
			// Arrange:
			TestContext context(notifyMode, Amount(1000));
			auto sender = test::GenerateRandomByteArray<Address>();
			auto senderAccountStateIter = context.addAccount(sender, Amount(999));

			// Act:
			context.update(sender);

			// Assert: no bucket was created
			const auto& activityBucket = senderAccountStateIter.get().ActivityBuckets.get(Importance_Height);
			EXPECT_EQ(model::ImportanceHeight(), activityBucket.StartHeight);
		}
	}

	TEST(TEST_CLASS, UpdateActivityBypassesUpdateOfAccountThatCannotHarvest_Commit) {
		AssertUpdateActivityBypassesUpdateOfAccountThatCannotHarvest(observers::NotifyMode::Commit);
	}

	TEST(TEST_CLASS, UpdateActivityBypassesUpdateOfAccountThatCannotHarvest_Rollback) {
		AssertUpdateActivityBypassesUpdateOfAccountThatCannotHarvest(observers::NotifyMode::Rollback);
	}

	// endregion

	// region basic update

	namespace {
		void AssertUpdateActivityUpdatesExistingBucket(observers::NotifyMode notifyMode, uint32_t expectedBeneficiaryCount) {
			// Arrange:
			TestContext context(notifyMode, Amount(1000));
			auto sender = test::GenerateRandomByteArray<Address>();
			auto senderAccountStateIter = context.addAccount(sender, Amount(1000));
			senderAccountStateIter.get().ActivityBuckets.update(Importance_Height, [](auto& bucket) {
				bucket.BeneficiaryCount = 100;
			});

			// Act:
			context.update(sender);

			// Assert: bucket was updated
			const auto& activityBucket = senderAccountStateIter.get().ActivityBuckets.get(Importance_Height);
			EXPECT_EQ(Importance_Height, activityBucket.StartHeight);
			EXPECT_EQ(expectedBeneficiaryCount, activityBucket.BeneficiaryCount);
		}
	}

	TEST(TEST_CLASS, UpdateActivityUpdatesExistingBucket_Commit) {
		AssertUpdateActivityUpdatesExistingBucket(observers::NotifyMode::Commit, 102);
	}

	TEST(TEST_CLASS, UpdateActivityUpdatesExistingBucket_Rollback) {
		AssertUpdateActivityUpdatesExistingBucket(observers::NotifyMode::Rollback, 98);
	}

	// endregion

	// region bucket creation

	namespace {
		void AssertUpdateActivityDoesNotCreateNewBucket(observers::NotifyMode notifyMode) {
			// Arrange:
			TestContext context(notifyMode, Amount(1000));
			auto sender = test::GenerateRandomByteArray<Address>();
			auto senderAccountStateIter = context.addAccount(sender, Amount(1000));

			// Act:
			context.update(sender);

			// Assert: bucket was not created
			const auto& activityBucket = senderAccountStateIter.get().ActivityBuckets.get(Importance_Height);
			EXPECT_EQ(model::ImportanceHeight(), activityBucket.StartHeight);
		}
	}

	TEST(TEST_CLASS, UpdateActivityCommitDoesNotCreateNewBucket) {
		AssertUpdateActivityDoesNotCreateNewBucket(observers::NotifyMode::Commit);
	}

	TEST(TEST_CLASS, UpdateActivityRollbackDoesNotCreateNewBucket) {
		AssertUpdateActivityDoesNotCreateNewBucket(observers::NotifyMode::Rollback);
	}

	// endregion

	// region bucket removal

	namespace {
		size_t CountNonzeroFields(const state::AccountActivityBuckets::ActivityBucket& activityBucket) {
			return (Amount() != activityBucket.TotalFeesPaid ? 1 : 0)
					+ (0u != activityBucket.BeneficiaryCount ? 1 : 0)
					+ (0u != activityBucket.RawScore ? 1 : 0);
		}

		void AssertUpdateActivityDoesNotRemoveZeroBucket(observers::NotifyMode notifyMode, uint32_t initialBeneficiaryCount) {
			// Arrange:
			TestContext context(notifyMode, Amount(1000));
			auto sender = test::GenerateRandomByteArray<Address>();
			auto senderAccountStateIter = context.addAccount(sender, Amount(1000));
			senderAccountStateIter.get().ActivityBuckets.update(Importance_Height, [initialBeneficiaryCount](auto& bucket) {
				bucket.BeneficiaryCount = initialBeneficiaryCount;
			});

			// Act:
			context.update(sender);

			// Assert: bucket was updated
			const auto& activityBucket = senderAccountStateIter.get().ActivityBuckets.get(Importance_Height);
			EXPECT_EQ(Importance_Height, activityBucket.StartHeight);
			EXPECT_EQ(0u, activityBucket.BeneficiaryCount);
			EXPECT_EQ(0u, CountNonzeroFields(activityBucket));
		}
	}

	TEST(TEST_CLASS, UpdateActivityCommitDoesNotRemoveZeroBucket) {
		auto initialBeneficiaryCount = std::numeric_limits<uint32_t>::max() - 1; // max - 1 + 2 == 0
		AssertUpdateActivityDoesNotRemoveZeroBucket(observers::NotifyMode::Commit, initialBeneficiaryCount);
	}

	TEST(TEST_CLASS, UpdateActivityRollbackDoesNotRemoveZeroBucket) {
		AssertUpdateActivityDoesNotRemoveZeroBucket(observers::NotifyMode::Rollback, 2);
	}

	namespace {
		void AssertUpdateActivityDoesNotRemoveNonzeroBucket(
				observers::NotifyMode notifyMode,
				uint32_t initialBeneficiaryCount,
				const char* message,
				const ActivityBucketConsumer& updateBucket) {
			// Arrange:
			TestContext context(notifyMode, Amount(1000));
			auto sender = test::GenerateRandomByteArray<Address>();
			auto senderAccountStateIter = context.addAccount(sender, Amount(1000));
			senderAccountStateIter.get().ActivityBuckets.update(Importance_Height, [initialBeneficiaryCount, updateBucket](auto& bucket) {
				bucket.BeneficiaryCount = initialBeneficiaryCount;
				updateBucket(bucket);
			});

			// Act:
			context.update(sender);

			// Assert: bucket was updated
			const auto& activityBucket = senderAccountStateIter.get().ActivityBuckets.get(Importance_Height);
			EXPECT_EQ(Importance_Height, activityBucket.StartHeight) << message;
			EXPECT_EQ(1u, CountNonzeroFields(activityBucket)) << message;
		}

		void AssertUpdateActivityDoesNotRemoveNonzeroBucketAll(observers::NotifyMode notifyMode, uint32_t initialBeneficiaryCount) {
			AssertUpdateActivityDoesNotRemoveNonzeroBucket(notifyMode, initialBeneficiaryCount, "TotalFeesPaid", [](auto& bucket) {
				bucket.TotalFeesPaid = bucket.TotalFeesPaid + Amount(1);
			});
			AssertUpdateActivityDoesNotRemoveNonzeroBucket(notifyMode, initialBeneficiaryCount, "BeneficiaryCount", [](auto& bucket) {
				++bucket.BeneficiaryCount;
			});
			AssertUpdateActivityDoesNotRemoveNonzeroBucket(notifyMode, initialBeneficiaryCount, "RawScore", [](auto& bucket) {
				++bucket.RawScore;
			});
		}
	}

	TEST(TEST_CLASS, UpdateActivityCommitDoesNotRemoveNonzeroBucket) {
		auto initialBeneficiaryCount = std::numeric_limits<uint32_t>::max() - 1; // max - 1 + 2 == 0
		AssertUpdateActivityDoesNotRemoveNonzeroBucketAll(observers::NotifyMode::Commit, initialBeneficiaryCount);
	}

	TEST(TEST_CLASS, UpdateActivityRollbackDoesNotRemoveNonzeroBucket) {
		AssertUpdateActivityDoesNotRemoveNonzeroBucketAll(observers::NotifyMode::Rollback, 2);
	}

	// endregion
}}
