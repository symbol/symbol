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

#include "catapult/cache_db/RocksPruningFilter.h"
#include "catapult/cache_db/RocksInclude.h"
#include "tests/catapult/cache_db/test/SliceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS RocksPruningFilterTests

	TEST(TEST_CLASS, CanDefaultCreateRocksPruningFilter) {
		// Arrange:
		RocksPruningFilter filter;

		// Assert:
		EXPECT_FALSE(!!filter.compactionFilter());
		EXPECT_EQ(0u, filter.pruningBoundary());
		EXPECT_EQ(0u, filter.numRemoved());
	}

	TEST(TEST_CLASS, CanCreateRocksPruningFilter) {
		// Arrange:
		RocksPruningFilter filter(FilterPruningMode::Enabled);

		// Assert:
		EXPECT_TRUE(!!filter.compactionFilter());
		EXPECT_EQ(0u, filter.pruningBoundary());
		EXPECT_EQ(0u, filter.numRemoved());
	}

	TEST(TEST_CLASS, SetPruningBoundaryIsNoOpWhenPruningIsDisabled) {
		// Arrange:
		RocksPruningFilter filter;

		// Act:
		filter.setPruningBoundary(1234);

		// Assert:
		EXPECT_EQ(0u, filter.pruningBoundary());
	}

	TEST(TEST_CLASS, SetPruningBoundaryChangesBoundaryWhenPruningIsEnabled) {
		// Arrange:
		RocksPruningFilter filter(FilterPruningMode::Enabled);

		// Act:
		filter.setPruningBoundary(1234);

		// Assert:
		EXPECT_EQ(1234u, filter.pruningBoundary());
	}

	namespace {
		bool RunFilter(rocksdb::CompactionFilter& filter, uint64_t value) {
			return filter.Filter(0, test::ToSlice(value), rocksdb::Slice(), nullptr, nullptr);
		}

		bool RunFilter(rocksdb::CompactionFilter& filter, const std::string& value) {
			return filter.Filter(0, value, rocksdb::Slice(), nullptr, nullptr);
		}
	}

	TEST(TEST_CLASS, CompactionFilterReturnsTrueForKeysSmallerThanBoundary) {
		// Arrange:
		RocksPruningFilter filter(FilterPruningMode::Enabled);
		filter.setPruningBoundary(10);
		auto& compactionFilter = *filter.compactionFilter();

		// Act + Assert:
		EXPECT_TRUE(RunFilter(compactionFilter, 0));
		EXPECT_TRUE(RunFilter(compactionFilter, 3));
		EXPECT_TRUE(RunFilter(compactionFilter, 5));
		EXPECT_TRUE(RunFilter(compactionFilter, 9));
	}

	TEST(TEST_CLASS, CompactionFilterReturnsFalseForKeysLargerOrEqualToBoundary) {
		// Arrange:
		RocksPruningFilter filter(FilterPruningMode::Enabled);
		filter.setPruningBoundary(10);
		auto& compactionFilter = *filter.compactionFilter();

		// Act + Assert:
		EXPECT_FALSE(RunFilter(compactionFilter, 10));
		for (auto i = 0u; i < 20; ++i)
			EXPECT_FALSE(RunFilter(compactionFilter, 10 + test::Random())) << " attempt " << i;
	}

	TEST(TEST_CLASS, CompactionFilterReturnsFalseForShortKeys) {
		// Arrange:
		RocksPruningFilter filter(FilterPruningMode::Enabled);
		filter.setPruningBoundary(std::numeric_limits<uint64_t>::max());
		auto& compactionFilter = *filter.compactionFilter();

		// Act + Assert:
		EXPECT_FALSE(RunFilter(compactionFilter, "size"));
		EXPECT_FALSE(RunFilter(compactionFilter, "size1"));
		EXPECT_FALSE(RunFilter(compactionFilter, "size12"));
		EXPECT_FALSE(RunFilter(compactionFilter, "size123"));

		EXPECT_TRUE(RunFilter(compactionFilter, "size1234"));
		EXPECT_TRUE(RunFilter(compactionFilter, "size12345"));
	}
}}
