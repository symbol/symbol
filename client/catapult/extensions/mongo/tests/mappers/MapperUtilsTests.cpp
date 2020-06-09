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

#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/model/EmbeddedTransaction.h"
#include "catapult/model/Receipt.h"
#include "catapult/model/VerifiableEntity.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/TestHarness.h"
#include <bsoncxx/exception/exception.hpp>

namespace catapult { namespace mongo { namespace mappers {

#define TEST_CLASS MapperUtilsTests

	// region to db type - ToBinary

	TEST(TEST_CLASS, CanConvertRawPointerToBinary) {
		// Arrange:
		uint8_t data[10];

		// Act
		auto bsonBinary = ToBinary(data, 10);

		// Assert:
		EXPECT_EQ(data, bsonBinary.bytes);
		EXPECT_EQ(10u, bsonBinary.size);
	}

	TEST(TEST_CLASS, CanConvertUnresolvedAddressToBinary) {
		// Arrange:
		auto unresolvedAddress = extensions::CopyToUnresolvedAddress(test::GenerateRandomByteArray<Address>());

		// Act
		auto bsonBinary = ToBinary(unresolvedAddress);

		// Assert:
		EXPECT_EQ(reinterpret_cast<const uint8_t*>(unresolvedAddress.data()), bsonBinary.bytes);
		EXPECT_EQ(Address::Size, bsonBinary.size);
	}

	namespace {
		template<size_t N>
		struct ByteArrayTag {
			static constexpr size_t Size = N;
		};

		template<size_t N>
		void AssertCanConvertByteArrayToBinary() {
			// Arrange:
			auto inputArray = utils::ByteArray<ByteArrayTag<N>>(test::GenerateRandomArray<N>());

			// Act
			auto bsonBinary = ToBinary(inputArray);

			// Assert:
			EXPECT_EQ(inputArray.data(), bsonBinary.bytes);
			EXPECT_EQ(N, bsonBinary.size);
		}
	}

	TEST(TEST_CLASS, CanConvertByteArraysToBinary) {
		AssertCanConvertByteArrayToBinary<123>();
		AssertCanConvertByteArrayToBinary<111>();
	}

	// endregion

	// region to db type - ToInt64 / ToInt32

	namespace {
		struct Uint64Traits {
			struct Custom_tag {};
			using Custom = utils::BaseValue<uint64_t, Custom_tag>;

			static constexpr auto Fitting_In_Signed_Int = 0x12345670'89ABCDEFull;
			static constexpr auto Not_Fitting_In_Signed_Int = 0x89ABCDEF'12345670ull;

			static auto ToInt(Custom baseValue) {
				return ToInt64(baseValue);
			}
		};

		struct Uint32Traits {
			struct Custom_tag {};
			using Custom = utils::BaseValue<uint32_t, Custom_tag>;

			static constexpr auto Fitting_In_Signed_Int = 0x12345670u;
			static constexpr auto Not_Fitting_In_Signed_Int = 0x89ABCDEFu;

			static auto ToInt(Custom baseValue) {
				return ToInt32(baseValue);
			}
		};
	}

#define TRAIT_BASED_TO_INT_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ToInt64) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<Uint64Traits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ToInt32) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<Uint32Traits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAIT_BASED_TO_INT_TEST(CanConvertBaseValueInSignedRange) {
		// Arrange:
		typename TTraits::Custom customValue(TTraits::Fitting_In_Signed_Int);

		// Act:
		auto result = TTraits::ToInt(customValue);

		// Assert:
		EXPECT_EQ(customValue.unwrap(), static_cast<typename TTraits::Custom::ValueType>(result));
	}

	TRAIT_BASED_TO_INT_TEST(CanConvertBaseValueInUnsignedRange) {
		// Arrange:
		typename TTraits::Custom customValue(TTraits::Not_Fitting_In_Signed_Int);

		// Act:
		auto result = TTraits::ToInt(customValue);

		// Assert:
		EXPECT_EQ(customValue.unwrap(), static_cast<typename TTraits::Custom::ValueType>(result));
	}

	// endregion

	// region from db type - ToUint32

	TEST(TEST_CLASS, CanConvertInt32ToUint32) {
		// Arrange: 2'147'483'647 is std::numeric_limits<int32_t>::max()
		std::vector<int32_t> values{ -1, 0, 1, 100, std::numeric_limits<int32_t>::max() };
		std::vector<uint32_t> expectedValues{ std::numeric_limits<uint32_t>::max(), 0, 1, 100, 2'147'483'647 };
		std::vector<uint32_t> actualValues;

		// Act:
		for (auto value : values)
			actualValues.push_back(ToUint32(value));

		// Assert:
		EXPECT_EQ(expectedValues, actualValues);
	}

	// endregion

	// region from db type - GetValue64

	TEST(TEST_CLASS, GetValue64_CanConvertInt64ToBaseValue) {
		// Arrange: serialize to mongo
		bson_stream::document builder;
		builder << "val" << static_cast<int64_t>(1234);
		auto view = builder.view();

		// Sanity:
		EXPECT_EQ(1u, test::GetFieldCount(view));

		// Act:
		auto value = GetValue64<Amount>(view["val"]);

		// Assert:
		EXPECT_EQ(Amount(1234), value);
	}

	TEST(TEST_CLASS, GetValue64_CannotConvertInt32ToBaseValue) {
		// Arrange: serialize to mongo
		bson_stream::document builder;
		builder << "val" << static_cast<int32_t>(1234);
		auto view = builder.view();

		// Sanity:
		EXPECT_EQ(1u, test::GetFieldCount(view));

		// Act + Assert:
		EXPECT_THROW(GetValue64<Amount>(view["val"]), bsoncxx::exception);
	}

	// endregion

	// region from db type - DbArrayToModelArray

	TEST(TEST_CLASS, DbBinaryToModelArray_CanMapBinaryToStlArray) {
		// Arrange:
		struct Tag {};
		auto input = utils::ByteArray<ByteArrayTag<17>>(test::GenerateRandomArray<17>());

		// Act: serialize to mongo
		bson_stream::document builder;
		builder << "bin" << ToBinary(input);
		auto view = builder.view();

		// Sanity:
		EXPECT_EQ(1u, test::GetFieldCount(view));
		auto dbBinary = view["bin"].get_binary();

		// Act:
		utils::ByteArray<ByteArrayTag<17>> output;
		DbBinaryToModelArray(output, dbBinary);

		// Assert:
		EXPECT_EQ(input, output);
	}

	namespace {
		template<size_t InputSize, size_t OutputSize>
		void AssertDbBinaryToModelArrayFailsIfOutputArrayHasUnexpectedSize() {
			// Arrange:
			auto input = utils::ByteArray<ByteArrayTag<InputSize>>(test::GenerateRandomArray<InputSize>());

			// Act: serialize to mongo
			bson_stream::document builder;
			builder << "bin" << ToBinary(input);
			auto view = builder.view();

			// Sanity:
			EXPECT_EQ(1u, test::GetFieldCount(view));
			auto dbBinary = view["bin"].get_binary();

			// Act + Assert:
			utils::ByteArray<ByteArrayTag<OutputSize>> output;
			EXPECT_THROW(DbBinaryToModelArray(output, dbBinary), catapult_invalid_argument);
		}
	}

	TEST(TEST_CLASS, DbBinaryToModelArray_FailsWhenOutputArrayIsTooSmall) {
		AssertDbBinaryToModelArrayFailsIfOutputArrayHasUnexpectedSize<17, 16>();
	}

	TEST(TEST_CLASS, DbBinaryToModelArray_FailsWhenOutputArrayIsTooLarge) {
		AssertDbBinaryToModelArrayFailsIfOutputArrayHasUnexpectedSize<17, 18>();
	}

	// endregion

	// region document helpers - GetUint64OrDefault

	TEST(TEST_CLASS, GetUint64OrDefault_ReturnsValueWhenValueTypeIsInt64) {
		// Arrange: serialize to mongo
		bson_stream::document builder;
		builder << "val" << static_cast<int64_t>(1234);
		auto view = builder.view();

		// Sanity:
		EXPECT_EQ(1u, test::GetFieldCount(view));

		// Act:
		auto value = GetUint64OrDefault(view, "val", 17);

		// Assert:
		EXPECT_EQ(1234u, value);
	}

	TEST(TEST_CLASS, GetUint64OrDefault_ReturnsDefaultValueWhenValueIsNotPresent) {
		// Arrange: serialize (nothing) to mongo
		bson_stream::document builder;
		auto view = builder.view();

		// Sanity:
		EXPECT_EQ(0u, test::GetFieldCount(view));

		// Act:
		auto value = GetUint64OrDefault(view, "val", 17);

		// Assert: default value is returned
		EXPECT_EQ(17u, value);
	}

	TEST(TEST_CLASS, GetUint64OrDefault_FailsWhenValueTypeIsInt32) {
		// Arrange: serialize to mongo
		bson_stream::document builder;
		builder << "val" << static_cast<int32_t>(1234);
		auto view = builder.view();

		// Sanity:
		EXPECT_EQ(1u, test::GetFieldCount(view));

		// Act + Assert:
		EXPECT_THROW(GetUint64OrDefault(view, "val", 17), bsoncxx::exception);
	}

	// endregion

	// region document helpers - IsEmptyDocument

	TEST(TEST_CLASS, IsEmptyDocument_ReturnsTrueWhenDocumentIsEmpty) {
		// Arrange:
		auto doc = bsoncxx::document::value(nullptr, 0, [](auto*) {});

		// Act:
		auto result = IsEmptyDocument(doc);

		// Assert:
		EXPECT_TRUE(result);
	}

	TEST(TEST_CLASS, IsEmptyDocument_ReturnsFalseWhenDocumentIsNotEmpty) {
		// Arrange:
		uint8_t value = 7;
		auto doc = bsoncxx::document::value(&value, 1, [](auto*) {});

		// Act:
		auto result = IsEmptyDocument(doc);

		// Assert:
		EXPECT_FALSE(result);
	}

	// endregion

	// region streaming helpers

	TEST(TEST_CLASS, CanStreamEmbeddedTransaction) {
		// Arrange:
		model::EmbeddedTransaction transaction;
		test::FillWithRandomData(transaction);

		// Act: serialize to mongo
		bson_stream::document builder;
		StreamEmbeddedTransaction(builder, transaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(4u, test::GetFieldCount(view));
		test::AssertEqualEmbeddedTransactionData(transaction, view);
	}

	TEST(TEST_CLASS, CanStreamVerifiableEntity) {
		// Arrange:
		model::VerifiableEntity entity;
		test::FillWithRandomData(entity);

		// Act: serialize to mongo
		bson_stream::document builder;
		StreamVerifiableEntity(builder, entity);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(6u, test::GetFieldCount(view));
		test::AssertEqualVerifiableEntityData(entity, view);
	}

	namespace {
		template<typename TMosaicId>
		void AssertCanStreamMosaic() {
			// Arrange:
			auto id = test::GenerateRandomValue<TMosaicId>();
			auto amount = test::GenerateRandomValue<Amount>();

			// Act: serialize to mongo
			bson_stream::document builder;
			auto mosaicsArray = builder << "arr" << bson_stream::open_array;
			StreamMosaic(mosaicsArray, id, amount);
			mosaicsArray << bson_stream::close_array;
			auto view = builder.view();

			// Assert:
			EXPECT_EQ(1u, test::GetFieldCount(view));

			auto dbArray = view["arr"].get_array().value;
			ASSERT_EQ(1u, test::GetFieldCount(dbArray));

			auto dbMosaic = dbArray.cbegin()->get_document().view();
			EXPECT_EQ(id, TMosaicId(test::GetUint64(dbMosaic, "id")));
			EXPECT_EQ(amount, Amount(test::GetUint64(dbMosaic, "amount")));
		}
	}

	TEST(TEST_CLASS, CanStreamMosaic) {
		AssertCanStreamMosaic<MosaicId>();
	}

	TEST(TEST_CLASS, CanStreamUnresolvedMosaic) {
		AssertCanStreamMosaic<UnresolvedMosaicId>();
	}

	TEST(TEST_CLASS, CanStreamReceipt) {
		// Arrange:
		model::Receipt receipt;
		test::FillWithRandomData({ reinterpret_cast<uint8_t*>(&receipt), sizeof(model::Receipt) });

		// Act: serialize to mongo
		bson_stream::document builder;
		StreamReceipt(builder, receipt);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(2u, test::GetFieldCount(view));
		test::AssertEqualReceiptData(receipt, view);
	}

	// endregion
}}}
