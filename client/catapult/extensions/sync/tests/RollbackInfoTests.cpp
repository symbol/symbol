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

#include "sync/src/RollbackInfo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace sync {

#define TEST_CLASS RollbackInfoTests

	namespace {
		struct Counters {
			size_t All;
			size_t Recent;
			size_t Longest;
		};

		void AssertInfo(const Counters& expectedCommitted, const Counters& expectedIgnored, const RollbackInfo& info) {
			// Assert:
			EXPECT_EQ(expectedCommitted.All, info.counter(RollbackResult::Committed, RollbackCounterType::All));
			EXPECT_EQ(expectedCommitted.Recent, info.counter(RollbackResult::Committed, RollbackCounterType::Recent));
			EXPECT_EQ(expectedCommitted.Longest, info.counter(RollbackResult::Committed, RollbackCounterType::Longest));
			EXPECT_EQ(expectedIgnored.All, info.counter(RollbackResult::Ignored, RollbackCounterType::All));
			EXPECT_EQ(expectedIgnored.Recent, info.counter(RollbackResult::Ignored, RollbackCounterType::Recent));
			EXPECT_EQ(expectedIgnored.Longest, info.counter(RollbackResult::Ignored, RollbackCounterType::Longest));
		}

		chain::TimeSupplier CreateTimeSupplier(const std::vector<Timestamp::ValueType>& times) {
			size_t index = 0;
			return [times, index]() mutable {
				auto timestamp = Timestamp(times[index]);
				if (index + 1 < times.size())
					++index;

				return timestamp;
			};
		}

		auto CreateDefaultRollbackInfo() {
			return RollbackInfo(CreateTimeSupplier({ 1 }), utils::TimeSpan::FromMinutes(10));
		}
	}

	TEST(TEST_CLASS, InfoIsInitiallyZeroInitialized) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Assert:
		AssertInfo({ 0, 0, 0 }, { 0, 0, 0 }, info);
	}

	namespace {
		void AddToInfo(RollbackInfo& info, size_t numBlocks) {
			for (auto i = 0u; i < numBlocks; ++i)
				info.increment();
		}
	}

	// region generic tests

	TEST(TEST_CLASS, IncrementDoesNotInfluenceStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		AddToInfo(info, 3);

		// Assert:
		AssertInfo({ 0, 0, 0 }, { 0, 0, 0 }, info);
	}

	TEST(TEST_CLASS, SaveAddsToCommittedStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		AddToInfo(info, 3);
		info.save();

		// Assert:
		AssertInfo({ 1, 1, 3 }, { 0, 0, 0 }, info);
	}

	TEST(TEST_CLASS, MulipleSavesAddToCommittedStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		AddToInfo(info, 3);
		info.save();
		AddToInfo(info, 5);
		info.save();
		AddToInfo(info, 4);
		info.save();

		// Assert:
		AssertInfo({ 3, 3, 5 }, { 0, 0, 0 }, info);
	}

	TEST(TEST_CLASS, ResetAddsToIgnoredStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		AddToInfo(info, 3);
		info.reset();

		// Assert:
		AssertInfo({ 0, 0, 0 }, { 1, 1, 3 }, info);
	}

	TEST(TEST_CLASS, MultipleResetsAddToIgnoredStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		AddToInfo(info, 3);
		info.reset();
		AddToInfo(info, 4);
		info.reset();
		AddToInfo(info, 5);
		info.reset();

		// Assert:
		AssertInfo({ 0, 0, 0 }, { 3, 3, 5 }, info);
	}

	TEST(TEST_CLASS, SavesAndResetsAddToProperStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		AddToInfo(info, 3);
		info.save();
		AddToInfo(info, 4);
		info.reset();
		AddToInfo(info, 5);
		info.save();
		AddToInfo(info, 6);
		info.reset();
		AddToInfo(info, 5);
		info.reset();

		// Assert:
		AssertInfo({ 2, 2, 5 }, { 3, 3, 6 }, info);
	}

	TEST(TEST_CLASS, ResetWithoutAddDoesNotInfluenceStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		AddToInfo(info, 3);
		info.save();
		info.reset();

		// Assert:
		AssertInfo({ 1, 1, 3 }, { 0, 0, 0 }, info);
	}

	TEST(TEST_CLASS, SaveWithoutAddDoesNotInfluenceStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		AddToInfo(info, 3);
		info.reset();
		info.save();

		// Assert:
		AssertInfo({ 0, 0, 0 }, { 1, 1, 3 }, info);
	}

	// endregion

	// region prune

	namespace {
		void PrepareTestData(RollbackInfo& info) {
			// Arrange:
			AddToInfo(info, 3);
			info.save();

			AddToInfo(info, 5);
			info.save();

			AddToInfo(info, 4);
			info.save();

			AddToInfo(info, 6);
			info.save();

			AddToInfo(info, 8);
			info.reset();

			AddToInfo(info, 9);
			info.reset();

			AddToInfo(info, 10);
			info.reset();
		}

		void AssertOperationPrunesRecentStats(const consumer<RollbackInfo&>& actInfo, const consumer<const RollbackInfo&>& assertInfo) {
			// Arrange: PrepareTestData will call the time supplier 7 + 7 times
			std::vector<Timestamp::ValueType> times(14, 1);
			times.push_back(10);
			RollbackInfo info(CreateTimeSupplier(times), utils::TimeSpan::FromMilliseconds(5));
			PrepareTestData(info);

			// Sanity:
			AssertInfo({ 4, 4, 6 }, { 3, 3, 10 }, info);

			// Act: time supplier will return 10 when saving / resetting the rollback info which will prune old infos
			AddToInfo(info, 12);
			actInfo(info);

			// Assert:
			assertInfo(info);
		}
	}

	TEST(TEST_CLASS, SavePrunesRecentStats) {
		AssertOperationPrunesRecentStats(
				// Act:
				[](auto& info) { info.save(); },
				// Assert: except for info added in line above, both recent stats should be reset
				[](const auto& info) { AssertInfo({ 5, 1, 12 }, { 3, 0, 10 }, info); });
	}

	TEST(TEST_CLASS, ResetPrunesRecentStats) {
		AssertOperationPrunesRecentStats(
				// Act:
				[](auto& info) { info.reset(); },

				// Assert: except for info added in line above, both recent stats should be reset
				[](const auto& info) { AssertInfo({ 4, 0, 6 }, { 4, 1, 12 }, info); });
	}

	// endregion
}}
