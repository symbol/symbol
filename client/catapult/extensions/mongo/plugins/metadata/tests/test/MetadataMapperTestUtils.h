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

#pragma once
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/namespace/src/types.h"
#include "catapult/utils/MemoryUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/test/nodeps/Random.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state { class MetadataEntry; } }

namespace catapult { namespace test {

	// region transaction traits

	/// Account metadata traits for tests.
	struct AccountMetadataTestTraits {
		/// Number of expected document fields.
		static constexpr size_t Expected_Field_Count = 4;

		/// Asserts additional data using \a transaction and \a view.
		template<typename TTransaction>
		static void AssertAdditionalData(const TTransaction&, const bsoncxx::document::view&)
		{}
	};

	/// Mosaic metadata traits for tests.
	struct MosaicMetadataTestTraits {
		/// Number of expected document fields.
		static constexpr size_t Expected_Field_Count = 5;

		/// Asserts additional data using \a transaction and \a view.
		template<typename TTransaction>
		static void AssertAdditionalData(const TTransaction& transaction, const bsoncxx::document::view& view) {
			EXPECT_EQ(transaction.TargetMosaicId, UnresolvedMosaicId(GetUint64(view, "targetMosaicId")));
		}
	};

	/// Namespace metadata traits for tests.
	struct NamespaceMetadataTestTraits {
		/// Number of expected document fields.
		static constexpr size_t Expected_Field_Count = 5;

		/// Asserts additional data using \a transaction and \a view.
		template<typename TTransaction>
		static void AssertAdditionalData(const TTransaction& transaction, const bsoncxx::document::view& view) {
			EXPECT_EQ(transaction.TargetNamespaceId, NamespaceId(GetUint64(view, "targetNamespaceId")));
		}
	};

	// endregion

	/// Asserts that the transaction with attachment of size \a valueSize can be mapped.
	template<typename TTraits, typename TTransactionTraits>
	void AssertCanMapTransaction(uint16_t valueSize) {
		// Arrange:
		using TransactionType = typename TTraits::TransactionType;
		auto entitySize = sizeof(TransactionType) + valueSize;
		auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
		FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });
		pTransaction->Size = static_cast<uint32_t>(entitySize);
		pTransaction->ValueSize = valueSize;

		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mongo::mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, *pTransaction);
		auto view = builder.view();

		// Assert:
		auto expectedFieldCount = TTransactionTraits::Expected_Field_Count;
		EXPECT_EQ(0 == valueSize ? expectedFieldCount : expectedFieldCount + 1, GetFieldCount(view));
		EXPECT_EQ(pTransaction->TargetAddress, GetUnresolvedAddressValue(view, "targetAddress"));
		EXPECT_EQ(pTransaction->ScopedMetadataKey, GetUint64(view, "scopedMetadataKey"));
		EXPECT_EQ(pTransaction->ValueSizeDelta, GetInt32(view, "valueSizeDelta"));
		EXPECT_EQ(pTransaction->ValueSize, GetUint32(view, "valueSize"));
		if (0 < valueSize) {
			auto dbValue = view["value"].get_binary();
			ASSERT_EQ(pTransaction->ValueSize, dbValue.size);
			EXPECT_EQ_MEMORY(pTransaction->ValuePtr(), dbValue.bytes, pTransaction->ValueSize);
		}

		TTransactionTraits::AssertAdditionalData(*pTransaction, view);
	}

	/// Asserts that model \a metadataEntry and dbmodel \a dbMetadataEntry are equal.
	void AssertEqualMetadataEntry(const state::MetadataEntry& metadataEntry, const bsoncxx::document::view& dbMetadataEntry);
}}
