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

#include "mongo/src/mappers/ReceiptMapper.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/mocks/MockReceiptMapper.h"
#include "tests/test/core/mocks/MockReceipt.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace mappers {

#define TEST_CLASS ReceiptMapperTests

	namespace {
		template<typename TReceipt>
		auto CreateDbDocument(const TReceipt& receipt) {
			bson_stream::document builder;
			StreamReceipt(builder, receipt);
			return builder << bsoncxx::builder::stream::finalize;
		}

		auto CreateDbDocument(const mocks::MockReceipt& receipt, const MongoReceiptRegistry& registry) {
			bson_stream::document builder;
			StreamReceipt(builder, receipt, registry);
			return builder << bsoncxx::builder::stream::finalize;
		}
	}

	// region StreamReceipt

	TEST(TEST_CLASS, CanStreamArtifactExpiryReceipt) {
		// Arrange:
		model::ArtifactExpiryReceipt<MosaicId> receipt(model::ReceiptType(), MosaicId(123));

		// Act:
		auto document = CreateDbDocument(receipt);

		// Assert:
		auto view = document.view();
		EXPECT_EQ(1u, test::GetFieldCount(view));

		EXPECT_EQ(123u, test::GetUint64(view, "artifactId"));
	}

	// endregion

	// region ToReceiptDbModel

	TEST(TEST_CLASS, CanStreamRegisteredReceipt) {
		// Arrange:
		mocks::MockReceipt receipt;
		receipt.Version = 5;
		receipt.Type = mocks::MockReceipt::Receipt_Type;
		receipt.Payload = { 1, 2, 3 };
		MongoReceiptRegistry registry;
		registry.registerPlugin(mocks::CreateMockReceiptMongoPlugin(utils::to_underlying_type(mocks::MockReceipt::Receipt_Type)));

		// Act:
		auto document = CreateDbDocument(receipt, registry);

		// Assert:
		auto view = document.view();
		EXPECT_EQ(3u, test::GetFieldCount(view));

		test::AssertEqualReceiptData(receipt, view);
		EXPECT_EQ(receipt.Payload, test::GetBinaryArray<11>(view, "mock_payload"));
	}

	TEST(TEST_CLASS, CanStreamUnknownReceipt) {
		// Arrange:
		mocks::MockReceipt receipt;
		receipt.Size = sizeof(mocks::MockReceipt);
		receipt.Version = 5;
		receipt.Type = mocks::MockReceipt::Receipt_Type;
		receipt.Payload = { 1, 2, 3 };
		MongoReceiptRegistry registry;

		// Act:
		auto document = CreateDbDocument(receipt, registry);

		// Assert:
		auto view = document.view();
		EXPECT_EQ(3u, test::GetFieldCount(view));

		test::AssertEqualReceiptData(receipt, view);
		EXPECT_EQ(receipt.Payload, test::GetBinaryArray<11>(view, "bin"));
	}

	// endregion
}}}
