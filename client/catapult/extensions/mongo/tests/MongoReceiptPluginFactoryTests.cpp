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

#include "mongo/src/MongoReceiptPluginFactory.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/test/core/mocks/MockReceipt.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo {

#define TEST_CLASS MongoReceiptPluginFactoryTests

	namespace {
		constexpr auto Mock_Receipt_Type = static_cast<model::ReceiptType>(0xFFFF);

		void Stream(bsoncxx::builder::stream::document& builder, const mocks::MockReceipt& receipt) {
			builder << "version0" << static_cast<int32_t>(receipt.Version);
		}

		static auto CreatePlugin() {
			return MongoReceiptPluginFactory::Create<mocks::MockReceipt>(mocks::MockReceipt::Receipt_Type, Stream);
		}
	}

	// region basic

	TEST(TEST_CLASS, CanCreatePlugin) {
		// Act:
		auto pPlugin = CreatePlugin();

		// Assert:
		EXPECT_EQ(Mock_Receipt_Type, pPlugin->type());
	}

	TEST(TEST_CLASS, CanStreamReceipt) {
		// Arrange:
		auto pPlugin = CreatePlugin();
		bsoncxx::builder::stream::document builder;

		mocks::MockReceipt receipt;
		receipt.Version = 0x57;

		// Act:
		pPlugin->streamReceipt(builder, receipt);
		auto dbReceipt = builder << bsoncxx::builder::stream::finalize;

		// Assert:
		auto view = dbReceipt.view();
		EXPECT_EQ(1u, test::GetFieldCount(view));

		EXPECT_EQ(0x57u, test::GetUint32(view, "version0"));
	}

	// endregion

	// region balance transfer

	TEST(TEST_CLASS, CreateBalanceTransferReceiptMongoPluginRespectsSuppliedType) {
		// Act:
		auto pPlugin = CreateBalanceTransferReceiptMongoPlugin(static_cast<model::ReceiptType>(1234));

		// Assert:
		EXPECT_EQ(static_cast<model::ReceiptType>(1234), pPlugin->type());
	}

	TEST(TEST_CLASS, CanStreamBalanceTransferReceipt) {
		// Arrange:
		auto pPlugin = CreateBalanceTransferReceiptMongoPlugin(model::ReceiptType());
		bsoncxx::builder::stream::document builder;

		model::BalanceTransferReceipt receipt(
				model::ReceiptType(),
				test::GenerateRandomByteArray<Address>(),
				test::GenerateRandomByteArray<Address>(),
				MosaicId(234),
				Amount(345));

		// Act:
		pPlugin->streamReceipt(builder, receipt);
		auto dbReceipt = builder << bsoncxx::builder::stream::finalize;

		// Assert:
		auto view = dbReceipt.view();
		EXPECT_EQ(4u, test::GetFieldCount(view));

		EXPECT_EQ(receipt.Mosaic.MosaicId, MosaicId(test::GetUint64(view, "mosaicId")));
		EXPECT_EQ(receipt.Mosaic.Amount, Amount(test::GetUint64(view, "amount")));
		EXPECT_EQ(receipt.SenderAddress, test::GetAddressValue(view, "senderAddress"));
		EXPECT_EQ(receipt.RecipientAddress, test::GetAddressValue(view, "recipientAddress"));
	}

	// endregion

	// region balance change

	TEST(TEST_CLASS, CreateBalanceChangeReceiptMongoPluginRespectsSuppliedType) {
		// Act:
		auto pPlugin = CreateBalanceChangeReceiptMongoPlugin(static_cast<model::ReceiptType>(1234));

		// Assert:
		EXPECT_EQ(static_cast<model::ReceiptType>(1234), pPlugin->type());
	}

	TEST(TEST_CLASS, CanStreamBalanceChangeReceipt) {
		// Arrange:
		auto pPlugin = CreateBalanceChangeReceiptMongoPlugin(model::ReceiptType());
		bsoncxx::builder::stream::document builder;

		model::BalanceChangeReceipt receipt(model::ReceiptType(), test::GenerateRandomByteArray<Address>(), MosaicId(234), Amount(345));

		// Act:
		pPlugin->streamReceipt(builder, receipt);
		auto dbReceipt = builder << bsoncxx::builder::stream::finalize;

		// Assert:
		auto view = dbReceipt.view();
		EXPECT_EQ(3u, test::GetFieldCount(view));

		EXPECT_EQ(receipt.Mosaic.MosaicId, MosaicId(test::GetUint64(view, "mosaicId")));
		EXPECT_EQ(receipt.Mosaic.Amount, Amount(test::GetUint64(view, "amount")));
		EXPECT_EQ(receipt.TargetAddress, test::GetAddressValue(view, "targetAddress"));
	}

	// endregion

	// region inflation

	TEST(TEST_CLASS, CreateInflationReceiptMongoPluginRespectsSuppliedType) {
		// Act:
		auto pPlugin = CreateInflationReceiptMongoPlugin(static_cast<model::ReceiptType>(1234));

		// Assert:
		EXPECT_EQ(static_cast<model::ReceiptType>(1234), pPlugin->type());
	}

	TEST(TEST_CLASS, CanStreamInflationReceipt) {
		// Arrange:
		auto pPlugin = CreateInflationReceiptMongoPlugin(model::ReceiptType());
		bsoncxx::builder::stream::document builder;

		model::InflationReceipt receipt(model::ReceiptType(), MosaicId(234), Amount(345));

		// Act:
		pPlugin->streamReceipt(builder, receipt);
		auto dbReceipt = builder << bsoncxx::builder::stream::finalize;

		// Assert:
		auto view = dbReceipt.view();
		EXPECT_EQ(2u, test::GetFieldCount(view));

		EXPECT_EQ(receipt.Mosaic.MosaicId, MosaicId(test::GetUint64(view, "mosaicId")));
		EXPECT_EQ(receipt.Mosaic.Amount, Amount(test::GetUint64(view, "amount")));
	}

	// endregion

	// region artifact expiry

	TEST(TEST_CLASS, CreateArtifactExpiryReceiptMongoPluginRespectsSuppliedType) {
		// Act:
		auto pPlugin = CreateArtifactExpiryReceiptMongoPlugin<MosaicId>(static_cast<model::ReceiptType>(1234));

		// Assert:
		EXPECT_EQ(static_cast<model::ReceiptType>(1234), pPlugin->type());
	}

	TEST(TEST_CLASS, CanStreamArtifactExpiryReceipt) {
		// Arrange:
		auto pPlugin = CreateArtifactExpiryReceiptMongoPlugin<MosaicId>(model::ReceiptType());
		bsoncxx::builder::stream::document builder;

		model::ArtifactExpiryReceipt<MosaicId> receipt(model::ReceiptType(), MosaicId(234));

		// Act:
		pPlugin->streamReceipt(builder, receipt);
		auto dbReceipt = builder << bsoncxx::builder::stream::finalize;

		// Assert:
		auto view = dbReceipt.view();
		EXPECT_EQ(1u, test::GetFieldCount(view));

		EXPECT_EQ(receipt.ArtifactId, MosaicId(test::GetUint64(view, "artifactId")));
	}

	// endregion
}}
