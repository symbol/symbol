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

#include "src/TransferMapper.h"
#include "sdk/src/builders/TransferBuilder.h"
#include "mongo/src/MongoTransactionPlugin.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTests.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS TransferMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS(Transfer)

		constexpr UnresolvedMosaicId Test_Mosaic_Id(1234);

		auto CreateTransferTransactionBuilder(
				const Key& signer,
				const UnresolvedAddress& recipient,
				const std::vector<uint8_t>& message,
				std::initializer_list<model::UnresolvedMosaic> mosaics) {
			builders::TransferBuilder builder(model::NetworkIdentifier::Private_Test, signer);
			builder.setRecipientAddress(recipient);

			if (!message.empty())
				builder.setMessage(message);

			for (const auto& mosaic : mosaics)
				builder.addMosaic(mosaic);

			return builder;
		}

		template<typename TTransaction>
		void AssertEqualNonInheritedTransferData(const TTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
			EXPECT_EQ(transaction.RecipientAddress, test::GetUnresolvedAddressValue(dbTransaction, "recipientAddress"));

			if (0 < transaction.MessageSize)
				EXPECT_EQ_MEMORY(transaction.MessagePtr(), test::GetBinary(dbTransaction, "message"), transaction.MessageSize);
			else
				EXPECT_FALSE(!!dbTransaction["message"].raw());

			auto dbMosaics = dbTransaction["mosaics"].get_array().value;
			ASSERT_EQ(transaction.MosaicsCount, test::GetFieldCount(dbMosaics));
			const auto* pMosaic = transaction.MosaicsPtr();
			auto iter = dbMosaics.cbegin();
			for (auto i = 0u; i < transaction.MosaicsCount; ++i) {
				EXPECT_EQ(pMosaic->MosaicId, UnresolvedMosaicId(test::GetUint64(iter->get_document().view(), "id")));
				EXPECT_EQ(pMosaic->Amount, Amount(test::GetUint64(iter->get_document().view(), "amount")));
				++pMosaic;
				++iter;
			}
		}

		template<typename TTraits>
		void AssertCanMapTransferTransaction(const std::vector<uint8_t>& message, std::initializer_list<model::UnresolvedMosaic> mosaics) {
			// Arrange:
			auto signer = test::GenerateRandomByteArray<Key>();
			auto recipient = test::GenerateRandomUnresolvedAddress();
			auto pTransaction = TTraits::Adapt(CreateTransferTransactionBuilder(signer, recipient, message, mosaics));
			auto pPlugin = TTraits::CreatePlugin();

			// Act:
			mappers::bson_stream::document builder;
			pPlugin->streamTransaction(builder, *pTransaction);
			auto view = builder.view();

			// Assert:
			EXPECT_EQ(message.empty() ? 2u : 3u, test::GetFieldCount(view));
			AssertEqualNonInheritedTransferData(*pTransaction, view);
		}
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , model::Entity_Type_Transfer)

	// region streamTransaction

	PLUGIN_TEST(CanMapTransferTransactionWithNeitherMessageNorMosaics) {
		AssertCanMapTransferTransaction<TTraits>({}, {});
	}

	PLUGIN_TEST(CanMapTransferTransactionWithTypeOnlyMessageButWithoutMosaics) {
		AssertCanMapTransferTransaction<TTraits>({ 0x48 }, {});
	}

	PLUGIN_TEST(CanMapTransferTransactionWithMessageButWithoutMosaics) {
		AssertCanMapTransferTransaction<TTraits>({ 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64 }, {});
	}

	PLUGIN_TEST(CanMapTransferTransactionWithoutMessageButWithSingleMosaic) {
		AssertCanMapTransferTransaction<TTraits>({}, { { Test_Mosaic_Id, Amount(234) } });
	}

	PLUGIN_TEST(CanMapTransferTransactionWithoutMessageButWithMultipleMosaics) {
		AssertCanMapTransferTransaction<TTraits>(
				{},
				{ { Test_Mosaic_Id, Amount(234) }, { UnresolvedMosaicId(1357), Amount(345) }, { UnresolvedMosaicId(31), Amount(45) } });
	}

	PLUGIN_TEST(CanMapTransferTransactionWithMessageAndSingleMosaic) {
		AssertCanMapTransferTransaction<TTraits>(
				{ 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64 },
				{ { Test_Mosaic_Id, Amount(234) } });
	}

	PLUGIN_TEST(CanMapTransferTransactionWithMessageAndMultipleMosaics) {
		AssertCanMapTransferTransaction<TTraits>(
				{ 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64 },
				{ { Test_Mosaic_Id, Amount(234) }, { UnresolvedMosaicId(1357), Amount(345) }, { UnresolvedMosaicId(31), Amount(45) } });
	}

	// endregion
}}}
