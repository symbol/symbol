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

#include "mongo/src/BulkWriteResult.h"
#include "tests/TestHarness.h"
#include <bsoncxx/builder/stream/document.hpp>

namespace catapult { namespace mongo {

#define TEST_CLASS BulkWriteResultTests

	namespace {
		BulkWriteResult CreateBulkWriteResultWithCounts(
				int32_t numInserted,
				int32_t numMatched,
				int32_t numModified,
				int32_t numDeleted,
				int32_t numUpserted) {
			using namespace bsoncxx::builder::stream;

			document builder;
			builder
					<< "nInserted" << numInserted
					<< "nMatched" << numMatched
					<< "nModified" << numModified
					<< "nRemoved" << numDeleted
					<< "nUpserted" << numUpserted;

			return BulkWriteResult(mongocxx::result::bulk_write(builder << finalize));
		}
	}

	TEST(TEST_CLASS, CanCreateZeroedResult) {
		// Act:
		BulkWriteResult result;

		// Assert:
		EXPECT_EQ(0, result.NumInserted);
		EXPECT_EQ(0, result.NumMatched);
		EXPECT_EQ(0, result.NumModified);
		EXPECT_EQ(0, result.NumDeleted);
		EXPECT_EQ(0, result.NumUpserted);
	}

	TEST(TEST_CLASS, CanCreateAroundRawMongoResult) {
		// Act:
		auto result = CreateBulkWriteResultWithCounts(1, 3, 2, 9, 8);

		// Assert:
		EXPECT_EQ(1, result.NumInserted);
		EXPECT_EQ(3, result.NumMatched);
		EXPECT_EQ(2, result.NumModified);
		EXPECT_EQ(9, result.NumDeleted);
		EXPECT_EQ(8, result.NumUpserted);
	}

	TEST(TEST_CLASS, CanAggregateMultipleResults) {
		// Arrange:
		std::vector<BulkWriteResult> results{
			CreateBulkWriteResultWithCounts(1, 3, 2, 9, 8),
			CreateBulkWriteResultWithCounts(5, 4, 3, 2, 1),
			CreateBulkWriteResultWithCounts(1, 1, 1, 1, 1)
		};

		// Act:
		auto result = BulkWriteResult::Aggregate(results);

		// Assert:
		EXPECT_EQ(7, result.NumInserted);
		EXPECT_EQ(8, result.NumMatched);
		EXPECT_EQ(6, result.NumModified);
		EXPECT_EQ(12, result.NumDeleted);
		EXPECT_EQ(10, result.NumUpserted);
	}
}}
