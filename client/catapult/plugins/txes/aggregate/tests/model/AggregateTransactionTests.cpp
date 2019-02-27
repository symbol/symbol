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
#include "tests/test/core/TransactionContainerTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS AggregateTransactionTests

	// region size + properties

	TEST(TEST_CLASS, EntityHasExpectedSize){
		// Arrange:
		auto expectedSize =
				sizeof(Transaction) // base
				+ sizeof(uint32_t); // payload size

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(AggregateTransaction));
		EXPECT_EQ(120u + 4, sizeof(AggregateTransaction));
	}

	TEST(TEST_CLASS, TransactionHasExpectedProperties) {
		// Assert:
		EXPECT_EQ(Entity_Type_Aggregate_Complete, AggregateTransaction::Entity_Type);
		EXPECT_EQ(2u, AggregateTransaction::Current_Version);
	}

	// endregion

	// region test utils

	namespace {
		using EmbeddedTransactionType = mocks::EmbeddedMockTransaction;

		std::unique_ptr<AggregateTransaction> CreateAggregateTransaction(
				uint32_t extraSize,
				std::initializer_list<uint16_t> attachmentExtraSizes) {
			uint32_t size = sizeof(AggregateTransaction) + extraSize;
			for (auto attachmentExtraSize : attachmentExtraSizes)
				size += sizeof(EmbeddedTransactionType) + attachmentExtraSize;

			auto pTransaction = utils::MakeUniqueWithSize<AggregateTransaction>(size);
			pTransaction->Size = size;
			pTransaction->PayloadSize = size - (sizeof(AggregateTransaction) + extraSize);

			auto* pData = reinterpret_cast<uint8_t*>(pTransaction.get() + 1);
			for (auto attachmentExtraSize : attachmentExtraSizes) {
				auto pEmbeddedTransaction = reinterpret_cast<EmbeddedTransactionType*>(pData);
				pEmbeddedTransaction->Size = sizeof(EmbeddedTransactionType) + attachmentExtraSize;
				pEmbeddedTransaction->Type = EmbeddedTransactionType::Entity_Type;
				pEmbeddedTransaction->Data.Size = attachmentExtraSize;
				pData += pEmbeddedTransaction->Size;
			}

			return pTransaction;
		}

		EmbeddedTransactionType& GetSecondTransaction(AggregateTransaction& transaction) {
			uint8_t* pBytes = reinterpret_cast<uint8_t*>(transaction.TransactionsPtr());
			return *reinterpret_cast<EmbeddedTransactionType*>(pBytes + transaction.TransactionsPtr()->Size);
		}
	}

	// endregion

	// region transactions

	namespace {
		using ConstTraits = test::ConstTraitsT<AggregateTransaction>;
		using NonConstTraits = test::NonConstTraitsT<AggregateTransaction>;
	}

#define DATA_POINTER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Const) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ConstTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonConst) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NonConstTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	DATA_POINTER_TEST(TransactionsAreInaccessibleWhenAggregateHasNoTransactions) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, {});
		auto& accessor = TTraits::GetAccessor(*pTransaction);

		// Act + Assert:
		EXPECT_FALSE(!!accessor.TransactionsPtr());
		EXPECT_EQ(0u, test::CountTransactions(accessor.Transactions()));
	}

	DATA_POINTER_TEST(TransactionsAreAccessibleWhenAggregateHasTransactions) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });
		const auto* pTransactionEnd = test::AsVoidPointer(pTransaction.get() + 1);
		auto& accessor = TTraits::GetAccessor(*pTransaction);

		// Act + Assert:
		EXPECT_EQ(pTransactionEnd, accessor.TransactionsPtr());
		EXPECT_EQ(3u, test::CountTransactions(accessor.Transactions()));
	}

	// endregion

	// region cosignatures

	DATA_POINTER_TEST(CosignaturesAreInacessibleIfReportedSizeIsLessThanHeaderSize) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, {});
		--pTransaction->Size;
		auto& accessor = TTraits::GetAccessor(*pTransaction);

		// Act + Assert:
		EXPECT_EQ(0u, accessor.CosignaturesCount());
		EXPECT_FALSE(!!accessor.CosignaturesPtr());
	}

	DATA_POINTER_TEST(CosignaturesAreInacessibleIfThereAreNoCosignatures) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, {});
		auto& accessor = TTraits::GetAccessor(*pTransaction);

		// Act + Assert:
		EXPECT_EQ(0u, accessor.CosignaturesCount());
		EXPECT_FALSE(!!accessor.CosignaturesPtr());
	}

	DATA_POINTER_TEST(CosignaturesAreAccessibleIfThereAreNoTransactionsButCosignatures) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(2 * sizeof(Cosignature), {});
		const auto* pAggregateEnd = reinterpret_cast<const uint8_t*>(pTransaction.get() + 1);
		const auto* pTransactionsEnd = test::AsVoidPointer(pAggregateEnd + pTransaction->PayloadSize);
		auto& accessor = TTraits::GetAccessor(*pTransaction);

		// Act + Assert:
		EXPECT_EQ(2u, accessor.CosignaturesCount());
		EXPECT_EQ(pTransactionsEnd, accessor.CosignaturesPtr());

		// Sanity:
		EXPECT_EQ(pAggregateEnd, pTransactionsEnd);
	}

	DATA_POINTER_TEST(CosignaturesAreAccessibleIfThereAreTransactionsAndCosignatures) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(sizeof(Cosignature), { 1, 2, 3 });
		const auto* pAggregateEnd = reinterpret_cast<const uint8_t*>(pTransaction.get() + 1);
		const auto* pTransactionsEnd = test::AsVoidPointer(pAggregateEnd + pTransaction->PayloadSize);
		auto& accessor = TTraits::GetAccessor(*pTransaction);

		// Act + Assert:
		EXPECT_EQ(1u, accessor.CosignaturesCount());
		EXPECT_EQ(pTransactionsEnd, accessor.CosignaturesPtr());

		// Sanity:
		EXPECT_NE(pAggregateEnd, pTransactionsEnd);
	}

	DATA_POINTER_TEST(CosignaturesAreAccessibleIfThereAreTransactionsAndPartialCosignatures) {
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

	TEST(TEST_CLASS, SizeInvalidIfReportedSizeIsZero) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, {});
		pTransaction->Size = 0;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeInvalidIfReportedSizeIsLessThanHeaderSize) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, {});
		--pTransaction->Size;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeValidIfReportedSizeIsEqualToHeaderSize) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, {});

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pTransaction));
	}

	// endregion

	// region IsSizeValid - invalid inner tx sizes

	TEST(TEST_CLASS, SizeInvalidIfAnyTransactionHasPartialHeader) {
		// Arrange: create an aggregate with 1 extra byte (which can be interpeted as a partial tx header)
		auto pTransaction = CreateAggregateTransaction(1, {});

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeInvalidIfAnyTransactionHasInvalidSize) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });
		GetSecondTransaction(*pTransaction).Data.Size = 1;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeInvalidIfAnyTransactionHasZeroSize) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });
		GetSecondTransaction(*pTransaction).Size = 0;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeInvalidIfAnyInnerTransactionExpandsBeyondBuffer) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });
		GetSecondTransaction(*pTransaction).Size = pTransaction->Size - pTransaction->TransactionsPtr()->Size + 1;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	// endregion

	// region IsSizeValid - invalid inner tx types

	TEST(TEST_CLASS, SizeInvalidIfAnyTransactionHasUnknownType) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });
		GetSecondTransaction(*pTransaction).Type = static_cast<EntityType>(-1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeInvalidIfAnyTransactionDoesNotSupportEmbedding) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction, mocks::PluginOptionFlags::Not_Embeddable));
	}

	// endregion

	// region IsSizeValid - payload size

	TEST(TEST_CLASS, SizeInvalidIfPayloadSizeIsTooLargeRelativeToSize) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });
		++pTransaction->PayloadSize;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeInvalidIfSpaceForCosignaturesIsNotMultipleOfCosignatureSize) {
		// Arrange:
		for (auto extraSize : { 1u, 3u, static_cast<uint32_t>(sizeof(Cosignature) - 1) }) {
			// - add extra bytes, which will cause space to not be multiple of cosignature size
			auto pTransaction = CreateAggregateTransaction(2 * sizeof(Cosignature) + extraSize, { 1, 2, 3 });

			// Act + Assert:
			EXPECT_FALSE(IsSizeValid(*pTransaction)) << "extra size: " << extraSize;
		}
	}

	// endregion

	// region IsSizeValid - valid transactions

	TEST(TEST_CLASS, SizeValidIfReportedSizeIsEqualToHeaderSizePlusTransactionsSize) {
		// Arrange:
		auto pTransaction = CreateAggregateTransaction(0, { 1, 2, 3 });

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeValidIfReportedSizeIsEqualToHeaderSizePlusTransactionsSizeAndHasSpaceForCosignatures) {
		// Arrange:
		for (auto numCosignatures : { 1u, 3u }) {
			// Arrange:
			auto pTransaction = CreateAggregateTransaction(numCosignatures * sizeof(Cosignature), { 1, 2, 3 });

			// Act + Assert:
			EXPECT_TRUE(IsSizeValid(*pTransaction)) << "num cosignatures: " << numCosignatures;
		}
	}

	// endregion
}}
