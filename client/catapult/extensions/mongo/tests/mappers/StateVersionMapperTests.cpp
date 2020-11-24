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

#include "mongo/src/mappers/StateVersionMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace mappers {

#define TEST_CLASS StateVersionMapperTests

	TEST(TEST_CLASS, CanAddMetaSubDocumentWithVersion) {
		// Arrange:
		bson_stream::document builder;
		auto modelDoc = builder
				<< "foo" << bson_stream::open_document
					<< "bar" << 12
					<< "baz" << 23
				<< bson_stream::close_document
				<< bson_stream::finalize;

		// Act:
		auto doc = AddStateVersion(modelDoc, 15);

		// Assert:
		auto view = doc.view();
		EXPECT_EQ(2u, test::GetFieldCount(view));

		auto fooView = view["foo"].get_document().view();
		EXPECT_EQ(2u, test::GetFieldCount(fooView));
		EXPECT_EQ(12u, test::GetUint32(fooView, "bar"));
		EXPECT_EQ(23u, test::GetUint32(fooView, "baz"));

		auto metaView = view["meta"].get_document().view();
		EXPECT_EQ(1u, test::GetFieldCount(metaView));
		EXPECT_EQ(15u, test::GetUint32(metaView, "version"));
	}

	TEST(TEST_CLASS, CanMergeVersionIntoExistingMetaSubDocument) {
		// Arrange:
		bson_stream::document builder;
		auto modelDoc = builder
				<< "foo" << bson_stream::open_document
					<< "bar" << 12
					<< "baz" << 23
				<< bson_stream::close_document
				<< "meta" << bson_stream::open_document
					<< "abc" << 77
					<< "xyz" << 11
				<< bson_stream::close_document
				<< bson_stream::finalize;

		// Act:
		auto doc = AddStateVersion(modelDoc, 15);

		// Assert:
		auto view = doc.view();
		EXPECT_EQ(2u, test::GetFieldCount(view));

		auto fooView = view["foo"].get_document().view();
		EXPECT_EQ(2u, test::GetFieldCount(fooView));
		EXPECT_EQ(12u, test::GetUint32(fooView, "bar"));
		EXPECT_EQ(23u, test::GetUint32(fooView, "baz"));

		auto metaView = view["meta"].get_document().view();
		EXPECT_EQ(3u, test::GetFieldCount(metaView));
		EXPECT_EQ(77u, test::GetUint32(metaView, "abc"));
		EXPECT_EQ(11u, test::GetUint32(metaView, "xyz"));
		EXPECT_EQ(15u, test::GetUint32(metaView, "version"));
	}

	TEST(TEST_CLASS, CanOverwriteVersionWhenPresentInExistingMetaSubDocument) {
		// Arrange:
		bson_stream::document builder;
		auto modelDoc = builder
				<< "foo" << bson_stream::open_document
					<< "bar" << 12
					<< "baz" << 23
				<< bson_stream::close_document
				<< "meta" << bson_stream::open_document
					<< "abc" << 77
					<< "xyz" << 11
					<< "version" << 33
				<< bson_stream::close_document
				<< bson_stream::finalize;

		// Act:
		auto doc = AddStateVersion(modelDoc, 15);

		// Assert:
		auto view = doc.view();
		EXPECT_EQ(2u, test::GetFieldCount(view));

		auto fooView = view["foo"].get_document().view();
		EXPECT_EQ(2u, test::GetFieldCount(fooView));
		EXPECT_EQ(12u, test::GetUint32(fooView, "bar"));
		EXPECT_EQ(23u, test::GetUint32(fooView, "baz"));

		auto metaView = view["meta"].get_document().view();
		EXPECT_EQ(3u, test::GetFieldCount(metaView));
		EXPECT_EQ(77u, test::GetUint32(metaView, "abc"));
		EXPECT_EQ(11u, test::GetUint32(metaView, "xyz"));
		EXPECT_EQ(15u, test::GetUint32(metaView, "version"));
	}
}}}
