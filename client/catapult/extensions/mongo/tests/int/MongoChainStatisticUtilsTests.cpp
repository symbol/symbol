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

#include "mongo/src/MongoChainStatisticUtils.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

#define TEST_CLASS MongoChainStatisticUtilsTests

	namespace {
		template<typename TAction>
		void RunTestWithDatabase(TAction action) {
			// Arrange:
			test::PrepareDatabase(test::DatabaseName());
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];

			// Act + Assert:
			action(database);
		}

		auto CreateUpdateDocument(int64_t value) {
			return document() << "$set" << open_document << "value" << value << close_document << finalize;
		}
	}

	TEST(TEST_CLASS, ChainStatisticDocumentIsEmptyWhenNotSet) {
		// Arrange:
		RunTestWithDatabase([](auto& database) {
			// Act:
			auto doc = GetChainStatisticDocument(database);

			// Assert:
			EXPECT_TRUE(mappers::IsEmptyDocument(doc));
		});
	}

	TEST(TEST_CLASS, CanSetChainStatisticDocument) {
		// Arrange:
		RunTestWithDatabase([](auto& database) {
			// Act:
			auto result = TrySetChainStatisticDocument(database, CreateUpdateDocument(12).view());
			auto doc = GetChainStatisticDocument(database);

			// Assert:
			EXPECT_EQ(1, result.NumUpserted);
			EXPECT_EQ(0, result.NumModified);

			EXPECT_FALSE(mappers::IsEmptyDocument(doc));
			EXPECT_EQ(12u, test::GetUint64(doc.view(), "value"));
		});
	}

	TEST(TEST_CLASS, CanUpdateChainStatisticDocument) {
		// Arrange:
		RunTestWithDatabase([](auto& database) {
			TrySetChainStatisticDocument(database, CreateUpdateDocument(12).view());

			// Act:
			auto result = TrySetChainStatisticDocument(database, CreateUpdateDocument(15).view());
			auto doc = GetChainStatisticDocument(database);

			// Assert:
			EXPECT_EQ(0, result.NumUpserted);
			EXPECT_EQ(1, result.NumModified);

			EXPECT_FALSE(mappers::IsEmptyDocument(doc));
			EXPECT_EQ(15u, test::GetUint64(doc.view(), "value"));
		});
	}

	TEST(TEST_CLASS, CanUpdateChainStatisticDocumentToSameValue) {
		// Arrange:
		RunTestWithDatabase([](auto& database) {
			TrySetChainStatisticDocument(database, CreateUpdateDocument(12).view());

			// Act:
			auto result = TrySetChainStatisticDocument(database, CreateUpdateDocument(12).view());
			auto doc = GetChainStatisticDocument(database);

			// Assert:
			EXPECT_EQ(0, result.NumUpserted);
			EXPECT_EQ(0, result.NumModified);

			EXPECT_FALSE(mappers::IsEmptyDocument(doc));
			EXPECT_EQ(12u, test::GetUint64(doc.view(), "value"));
		});
	}
}}
