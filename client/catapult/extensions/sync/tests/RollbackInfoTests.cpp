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

#include "sync/src/RollbackInfo.h"
#include "tests/test/nodeps/TimeSupplier.h"
#include "tests/TestHarness.h"

namespace catapult { namespace sync {

#define TEST_CLASS RollbackInfoTests

	namespace {
		struct Counters {
			size_t All;
			size_t Recent;
			size_t Longest;
		};

		void AddToInfo(RollbackInfoModifier& modifier, size_t numBlocks) {
			for (auto i = 0u; i < numBlocks; ++i)
				modifier.increment();
		}

		void AssertInfo(const Counters& expectedCommitted, const Counters& expectedIgnored, const RollbackInfoView& view) {
			// Assert:
			EXPECT_EQ(expectedCommitted.All, view.counter(RollbackResult::Committed, RollbackCounterType::All));
			EXPECT_EQ(expectedCommitted.Recent, view.counter(RollbackResult::Committed, RollbackCounterType::Recent));
			EXPECT_EQ(expectedCommitted.Longest, view.counter(RollbackResult::Committed, RollbackCounterType::Longest));
			EXPECT_EQ(expectedIgnored.All, view.counter(RollbackResult::Ignored, RollbackCounterType::All));
			EXPECT_EQ(expectedIgnored.Recent, view.counter(RollbackResult::Ignored, RollbackCounterType::Recent));
			EXPECT_EQ(expectedIgnored.Longest, view.counter(RollbackResult::Ignored, RollbackCounterType::Longest));
		}

		auto CreateDefaultRollbackInfo() {
			return RollbackInfo(test::CreateTimeSupplierFromMilliseconds({ 1 }), utils::TimeSpan::FromMinutes(10));
		}
	}

	// region constructor

	TEST(TEST_CLASS, InfoIsInitiallyZeroInitialized) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Assert:
		AssertInfo({ 0, 0, 0 }, { 0, 0, 0 }, info.view());
	}

	// endregion

	// region generic tests

	TEST(TEST_CLASS, IncrementDoesNotInfluenceStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		{
			auto modifier = info.modifier();
			AddToInfo(modifier, 3);
		}

		// Assert:
		AssertInfo({ 0, 0, 0 }, { 0, 0, 0 }, info.view());
	}

	TEST(TEST_CLASS, SaveAddsToCommittedStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		{
			auto modifier = info.modifier();
			AddToInfo(modifier, 3);
			modifier.save();
		}

		// Assert:
		AssertInfo({ 1, 1, 3 }, { 0, 0, 0 }, info.view());
	}

	TEST(TEST_CLASS, MulipleSavesAddToCommittedStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		{
			auto modifier = info.modifier();
			AddToInfo(modifier, 3);
			modifier.save();
			AddToInfo(modifier, 5);
			modifier.save();
			AddToInfo(modifier, 4);
			modifier.save();
		}

		// Assert:
		AssertInfo({ 3, 3, 5 }, { 0, 0, 0 }, info.view());
	}

	TEST(TEST_CLASS, ResetAddsToIgnoredStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		{
			auto modifier = info.modifier();
			AddToInfo(modifier, 3);
			modifier.reset();
		}

		// Assert:
		AssertInfo({ 0, 0, 0 }, { 1, 1, 3 }, info.view());
	}

	TEST(TEST_CLASS, MultipleResetsAddToIgnoredStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		{
			auto modifier = info.modifier();
			AddToInfo(modifier, 3);
			modifier.reset();
			AddToInfo(modifier, 4);
			modifier.reset();
			AddToInfo(modifier, 5);
			modifier.reset();
		}

		// Assert:
		AssertInfo({ 0, 0, 0 }, { 3, 3, 5 }, info.view());
	}

	TEST(TEST_CLASS, SavesAndResetsAddToProperStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		{
			auto modifier = info.modifier();
			AddToInfo(modifier, 3);
			modifier.save();
			AddToInfo(modifier, 4);
			modifier.reset();
			AddToInfo(modifier, 5);
			modifier.save();
			AddToInfo(modifier, 6);
			modifier.reset();
			AddToInfo(modifier, 5);
			modifier.reset();
		}

		// Assert:
		AssertInfo({ 2, 2, 5 }, { 3, 3, 6 }, info.view());
	}

	TEST(TEST_CLASS, ResetWithoutAddDoesNotInfluenceStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		{
			auto modifier = info.modifier();
			AddToInfo(modifier, 3);
			modifier.save();
			modifier.reset();
		}

		// Assert:
		AssertInfo({ 1, 1, 3 }, { 0, 0, 0 }, info.view());
	}

	TEST(TEST_CLASS, SaveWithoutAddDoesNotInfluenceStats) {
		// Arrange:
		auto info = CreateDefaultRollbackInfo();

		// Act:
		{
			auto modifier = info.modifier();
			AddToInfo(modifier, 3);
			modifier.reset();
			modifier.save();
		}

		// Assert:
		AssertInfo({ 0, 0, 0 }, { 1, 1, 3 }, info.view());
	}

	// endregion

	// region prune

	namespace {
		void PrepareTestData(RollbackInfoModifier&& modifier) {
			// Arrange:
			AddToInfo(modifier, 3);
			modifier.save();

			AddToInfo(modifier, 5);
			modifier.save();

			AddToInfo(modifier, 4);
			modifier.save();

			AddToInfo(modifier, 6);
			modifier.save();

			AddToInfo(modifier, 8);
			modifier.reset();

			AddToInfo(modifier, 9);
			modifier.reset();

			AddToInfo(modifier, 10);
			modifier.reset();
		}

		void AssertOperationPrunesRecentStats(
				const consumer<RollbackInfoModifier&>& actInfo,
				const consumer<const RollbackInfoView&>& assertInfo) {
			// Arrange: PrepareTestData will call the time supplier 7 + 7 times
			std::vector<uint32_t> rawTimestamps(14, 1);
			rawTimestamps.push_back(10);
			RollbackInfo info(test::CreateTimeSupplierFromMilliseconds(rawTimestamps), utils::TimeSpan::FromMilliseconds(5));
			PrepareTestData(info.modifier());

			// Sanity:
			AssertInfo({ 4, 4, 6 }, { 3, 3, 10 }, info.view());

			// Act: time supplier will return 10 when saving / resetting the rollback info which will prune old infos
			{
				auto modifier = info.modifier();
				AddToInfo(modifier, 12);
				actInfo(modifier);
			}

			// Assert:
			assertInfo(info.view());
		}
	}

	TEST(TEST_CLASS, SavePrunesRecentStats) {
		AssertOperationPrunesRecentStats(
				// Act:
				[](auto& modifier) { modifier.save(); },
				// Assert: except for info added in line above, both recent stats should be reset
				[](const auto& view) { AssertInfo({ 5, 1, 12 }, { 3, 0, 10 }, view); });
	}

	TEST(TEST_CLASS, ResetPrunesRecentStats) {
		AssertOperationPrunesRecentStats(
				// Act:
				[](auto& modifier) { modifier.reset(); },
				// Assert: except for info added in line above, both recent stats should be reset
				[](const auto& view) { AssertInfo({ 4, 0, 6 }, { 4, 1, 12 }, view); });
	}

	// endregion
}}
