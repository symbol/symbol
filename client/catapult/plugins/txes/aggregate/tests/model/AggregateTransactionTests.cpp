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

#include "src/model/AggregateTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/SizePrefixedEntityContainerTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS AggregateTransactionTests

	// region size + alignment + properties

#define TRANSACTION_FIELDS FIELD(TransactionsHash) FIELD(PayloadSize)

	TEST(TEST_CLASS, TransactionHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(Transaction) + sizeof(uint32_t);

#define FIELD(X) expectedSize += SizeOf32<decltype(AggregateTransaction::X)>();
		TRANSACTION_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(AggregateTransaction));
		EXPECT_EQ(128u + 4 + 36, sizeof(AggregateTransaction));
	}

	TEST(TEST_CLASS, TransactionHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(AggregateTransaction, X);
		TRANSACTION_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(AggregateTransaction) % 8);
	}

	TEST(TEST_CLASS, TransactionHasExpectedProperties) {
		EXPECT_EQ(Entity_Type_Aggregate_Complete, AggregateTransaction::Entity_Type);
		EXPECT_EQ(1u, AggregateTransaction::Current_Version);
	}

#undef TRANSACTION_FIELDS

	// endregion

	// region test utils

	namespace {
		using EmbeddedTransactionType = mocks::EmbeddedMockTransaction;

		uint32_t CalculateAggregateTransactionSize(uint32_t extraSize, std::initializer_list<uint16_t> attachmentExtraSizes) {
			uint32_t size = SizeOf32<AggregateTransaction>() + extraSize;
			for (auto attachmentExtraSize : attachmentExtraSizes) {
				uint32_t attachmentSize = SizeOf32<EmbeddedTransactionType>() + attachmentExtraSize;
				uint32_t paddingSize = utils::GetPaddingSize(attachmentSize, 8);
				size += attachmentSize + paddingSize;
			}

			return size;
		}

		std::unique_ptr<AggregateTransaction> CreateAggregateTransaction(
				uint32_t extraSize,
				std::initializer_list<uint16_t> attachmentExtraSizes) {
			auto size = CalculateAggregateTransactionSize(extraSize, attachmentExtraSizes);

			auto pTransaction = utils::MakeUniqueWithSize<AggregateTransaction>(size);
			pTransaction->Size = size;
			pTransaction->PayloadSize = size - (SizeOf32<AggregateTransaction>() + extraSize);

			auto* pData = reinterpret_cast<uint8_t*>(pTransaction.get() + 1);
			for (auto attachmentExtraSize : attachmentExtraSizes) {
				auto pEmbeddedTransaction = reinterpret_cast<EmbeddedTransactionType*>(pData);
				pEmbeddedTransaction->Size = SizeOf32<EmbeddedTransactionType>() + attachmentExtraSize;
				pEmbeddedTransaction->Type = EmbeddedTransactionType::Entity_Type;
				pEmbeddedTransaction->Data.Size = attachmentExtraSize;
				pData += pEmbeddedTransaction->Size + utils::GetPaddingSize(pEmbeddedTransaction->Size, 8);
			}

			return pTransaction;
		}

		EmbeddedTransactionType& GetSecondTransaction(AggregateTransaction& transaction) {
			uint8_t* pBytes = reinterpret_cast<uint8_t*>(transaction.TransactionsPtr());
			auto firstTransactionSize = transaction.TransactionsPtr()->Size;
			auto paddingSize = utils::GetPaddingSize(firstTransactionSize, 8);
			return *reinterpret_cast<EmbeddedTransactionType*>(pBytes + firstTransactionSize + paddingSize);
		}
	}

	// endregion

	// region data pointer traits

	namespace {
		using ConstTraits = test::ConstTraitsT<AggregateTransaction>;
		using NonConstTraits = test::NonConstTraitsT<AggregateTransaction>;
	}

#define DATA_POINTER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Const) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ConstTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonConst) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NonConstTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region transaction + cosignature permutations

	DATA_POINTER_TEST(TransactionsAndCosignaturesAreInaccessibleWhenAggregateContainsNeither) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, {});
		auto& accessor = TTraits::GetAccessor(*pTransaction);

		// Act + Assert:
		EXPECT_EQ(0u, test::CountContainerEntities(accessor.Transactions()));
		EXPECT_FALSE(!!accessor.TransactionsPtr());

		EXPECT_EQ(0u, accessor.CosignaturesCount());
		EXPECT_FALSE(!!accessor.CosignaturesPtr());
	}

	DATA_POINTER_TEST(OnlyTransactionsAreAccessibleWhenAggregateOnlyContainsTransactions) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });
		const auto* pAggregateEnd = test::AsVoidPointer(pTransaction.get() + 1);
		auto& accessor = TTraits::GetAccessor(*pTransaction);

		// Act + Assert:
		EXPECT_EQ(3u, test::CountContainerEntities(accessor.Transactions()));
		EXPECT_EQ(pAggregateEnd, accessor.TransactionsPtr());

		EXPECT_EQ(0u, accessor.CosignaturesCount());
		EXPECT_FALSE(!!accessor.CosignaturesPtr());
	}

	DATA_POINTER_TEST(OnlyCosignaturesAreAccessibleWhenAggregateOnlyContainsCosignatures) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(2 * sizeof(Cosignature), {});
		const auto* pAggregateEnd = reinterpret_cast<const uint8_t*>(pTransaction.get() + 1);
		const auto* pTransactionsEnd = test::AsVoidPointer(pAggregateEnd + pTransaction->PayloadSize);
		auto& accessor = TTraits::GetAccessor(*pTransaction);

		// Act + Assert:
		EXPECT_EQ(0u, test::CountContainerEntities(accessor.Transactions()));
		EXPECT_FALSE(!!accessor.TransactionsPtr());

		EXPECT_EQ(2u, accessor.CosignaturesCount());
		EXPECT_EQ(pTransactionsEnd, accessor.CosignaturesPtr());

		// Sanity:
		EXPECT_EQ(pAggregateEnd, pTransactionsEnd);
	}

	DATA_POINTER_TEST(TransactionsAndCosignaturesAreAccessibleWhenAggregateContainsBoth) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(sizeof(Cosignature), { 1, 2, 3 });
		const auto* pAggregateEnd = reinterpret_cast<const uint8_t*>(pTransaction.get() + 1);
		const auto* pTransactionsEnd = test::AsVoidPointer(pAggregateEnd + pTransaction->PayloadSize);
		auto& accessor = TTraits::GetAccessor(*pTransaction);

		// Act + Assert:
		EXPECT_EQ(3u, test::CountContainerEntities(accessor.Transactions()));
		EXPECT_EQ(test::AsVoidPointer(pAggregateEnd), accessor.TransactionsPtr());

		EXPECT_EQ(1u, accessor.CosignaturesCount());
		EXPECT_EQ(pTransactionsEnd, accessor.CosignaturesPtr());
	}

	// endregion

	// region cosignatures - edge cases

	DATA_POINTER_TEST(CosignaturesAreInacessibleWhenReportedSizeIsLessThanHeaderSize) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, {});
		--pTransaction->Size;
		auto& accessor = TTraits::GetAccessor(*pTransaction);

		// Act + Assert:
		EXPECT_EQ(0u, accessor.CosignaturesCount());
		EXPECT_FALSE(!!accessor.CosignaturesPtr());
	}

	DATA_POINTER_TEST(CosignaturesAreAccessibleWhenThereAreTransactionsAndPartialCosignatures) {
		// Arrange: three transactions and space for 2.5 cosignatures
		auto pTransaction = CreateAggregateTransaction(2 * sizeof(Cosignature) + sizeof(Cosignature) / 2, { 1, 2, 3 });
		const auto* pAggregateEnd = reinterpret_cast<const uint8_t*>(pTransaction.get() + 1);
		const auto* pTransactionsEnd = test::AsVoidPointer(pAggregateEnd + pTransaction->PayloadSize);
		auto& accessor = TTraits::GetAccessor(*pTransaction);

		// Act + Assert: two cosignatures should be exposed
		// - unlike in most transaction types, cosignature accessors assume PayloadSize is correct instead of returning nullptr
		// - this is because aggregate transaction size cannot be calculated without a registry, which is excessive for accessors
		EXPECT_EQ(2u, accessor.CosignaturesCount());
		EXPECT_EQ(pTransactionsEnd, accessor.CosignaturesPtr());

		// Sanity:
		EXPECT_NE(pAggregateEnd, pTransactionsEnd);
	}

	// endregion

	// region GetTransactionPayloadSize

	TEST(TEST_CLASS, GetTransactionPayloadSizeReturnsCorrectPayloadSize) {
		// Arrange:
		AggregateTransactionHeader header;
		header.PayloadSize = 123;

		// Act:
		auto payloadSize = GetTransactionPayloadSize(header);

		// Assert:
		EXPECT_EQ(123u, payloadSize);
	}

	// endregion

	namespace {
		bool IsSizeValid(const AggregateTransaction& transaction, mocks::PluginOptionFlags options = mocks::PluginOptionFlags::Default) {
			auto registry = mocks::CreateDefaultTransactionRegistry(options);
			return IsSizeValid(transaction, registry);
		}
	}

	// region IsSizeValid - no transactions

	TEST(TEST_CLASS, SizeInvalidWhenReportedSizeIsZero) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, {});
		pTransaction->Size = 0;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeInvalidWhenReportedSizeIsLessThanHeaderSize) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, {});
		--pTransaction->Size;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeValidWhenReportedSizeIsEqualToHeaderSize) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, {});

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pTransaction));
	}

	// endregion

	// region IsSizeValid - invalid inner tx sizes

	TEST(TEST_CLASS, SizeInvalidWhenAnyTransactionHasPartialHeader) {
		// Arrange: create an aggregate with 1 extra byte (which can be interpeted as a partial tx header)
		auto pTransaction = CreateAggregateTransaction(1, {});

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeInvalidWhenAnyTransactionHasInvalidSize) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });
		GetSecondTransaction(*pTransaction).Data.Size = 1;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeInvalidWhenAnyTransactionHasZeroSize) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });
		GetSecondTransaction(*pTransaction).Size = 0;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeInvalidWhenAnyInnerTransactionExpandsBeyondBuffer) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });
		GetSecondTransaction(*pTransaction).Size = pTransaction->Size - pTransaction->TransactionsPtr()->Size + 1;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	// endregion

	// region IsSizeValid - invalid inner tx types

	TEST(TEST_CLASS, SizeInvalidWhenAnyTransactionHasUnknownType) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });
		GetSecondTransaction(*pTransaction).Type = static_cast<EntityType>(std::numeric_limits<uint16_t>::max());

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeInvalidWhenAnyTransactionDoesNotSupportEmbedding) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction, mocks::PluginOptionFlags::Not_Embeddable));
	}

	// endregion

	// region IsSizeValid - payload size

	TEST(TEST_CLASS, SizeInvalidWhenPayloadSizeIsTooLargeRelativeToSize) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });
		pTransaction->PayloadSize += 8;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeInvalidWhenPayloadSizeIsNotPaddedProperly) {
		// Arrange: remove padding from last transaction
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });
		auto lastTxPaddingSize = utils::GetPaddingSize<uint32_t>(sizeof(EmbeddedTransactionType) + 3, 8);

		pTransaction->Size -= lastTxPaddingSize;
		pTransaction->PayloadSize -= lastTxPaddingSize;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeInvalidWhenSpaceForCosignaturesIsNotMultipleOfCosignatureSize) {
		// Arrange:
		for (auto extraSize : { 1u, 3u, SizeOf32<Cosignature>() - 1 }) {
			// - add extra bytes, which will cause space to not be multiple of cosignature size
			auto pTransaction = CreateAggregateTransaction(2 * SizeOf32<Cosignature>() + extraSize, { 1, 2, 3 });

			// Act + Assert:
			EXPECT_FALSE(IsSizeValid(*pTransaction)) << "extra size: " << extraSize;
		}
	}

	// endregion

	// region IsSizeValid - valid transactions

	TEST(TEST_CLASS, SizeValidWhenReportedSizeIsEqualToHeaderSizePlusTransactionsSize) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeValidWhenReportedSizeIsEqualToHeaderSizePlusTransactionsSizeAndHasSpaceForCosignatures) {
		// Arrange:
		for (auto numCosignatures : { 1u, 3u }) {
			// Arrange:
			auto pTransaction = CreateAggregateTransaction(numCosignatures * SizeOf32<Cosignature>(), { 1, 2, 3 });

			// Act + Assert:
			EXPECT_TRUE(IsSizeValid(*pTransaction)) << "num cosignatures: " << numCosignatures;
		}
	}

	// endregion
}}
