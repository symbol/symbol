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

#include "src/PropertyMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/property/src/model/AddressPropertyTransaction.h"
#include "plugins/txes/property/src/model/MosaicPropertyTransaction.h"
#include "plugins/txes/property/src/model/TransactionTypePropertyTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTestUtils.h"
#include "tests/test/PropertyCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS PropertyMapperTests

	/// Property mapper test suite.
	template<typename TTraits, typename TPropertyValueTraits>
	class PropertyMapperTests {
	public:
		static void AssertCanMapPropertyTransactionWithoutModifications() {
			// Assert:
			AssertCanMapPropertyTransaction(0);
		}

		static void AssertCanMapPropertyTransactionWithSingleModification() {
			// Assert:
			AssertCanMapPropertyTransaction(1);
		}

		static void AssertCanMapPropertyTransactionWithMultipleModifications() {
			// Assert:
			AssertCanMapPropertyTransaction(5);
		}

	private:
		static auto CreatePropertyTransaction(uint8_t numModifications) {
			using TransactionType = typename TTraits::TransactionType;
			auto valueSize = TPropertyValueTraits::PropertyValueSize();
			auto entitySize = static_cast<uint32_t>(sizeof(TransactionType) + numModifications * (1 + valueSize));
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });
			pTransaction->Size = entitySize;
			pTransaction->ModificationsCount = numModifications;
			return pTransaction;
		}

		template<typename TTransaction>
		static void AssertPropertyTransaction(const TTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
			EXPECT_EQ(transaction.PropertyType, static_cast<model::PropertyType>(test::GetUint8(dbTransaction, "propertyType")));

			auto dbModifications = dbTransaction["modifications"].get_array().value;
			ASSERT_EQ(transaction.ModificationsCount, test::GetFieldCount(dbModifications));
			const auto* pModification = transaction.ModificationsPtr();
			auto iter = dbModifications.cbegin();
			for (auto i = 0u; i < transaction.ModificationsCount; ++i) {
				auto view = iter->get_document().view();
				EXPECT_EQ(pModification->ModificationType, static_cast<model::PropertyModificationType>(test::GetUint8(view, "type")));

				RawBuffer buffer(view["value"].get_binary().bytes, view["value"].get_binary().size);
				EXPECT_EQ(pModification->Value, TPropertyValueTraits::FromBuffer(buffer));
				++pModification;
				++iter;
			}
		}

		static void AssertCanMapPropertyTransaction(uint8_t numModifications) {
			// Arrange:
			auto pTransaction = CreatePropertyTransaction(numModifications);
			auto pPlugin = TTraits::CreatePlugin();

			// Act:
			mappers::bson_stream::document builder;
			pPlugin->streamTransaction(builder, *pTransaction);
			auto view = builder.view();

			// Assert:
			EXPECT_EQ(2u, test::GetFieldCount(view));
			AssertPropertyTransaction(*pTransaction, view);
		}
	};

// additional template param is needed in order to handle property value correctly, so can't use PLUGIN_TEST
#define MAKE_PROPERTY_MAPPER_TEST(TRAITS, PREFIX, VALUE_TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##_##PREFIX##_##Regular) { \
		PropertyMapperTests<PREFIX##Regular##TRAITS, VALUE_TRAITS>::Assert##TEST_NAME(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_##PREFIX##_##Embedded) { \
		PropertyMapperTests<PREFIX##Embedded##TRAITS, VALUE_TRAITS>::Assert##TEST_NAME(); \
	}

#define DEFINE_PROPERTY_MAPPER_TESTS_WITH_PREFIXED_TRAITS(TRAITS, PREFIX, PROPERTY_VALUE_TRAITS) \
	MAKE_PROPERTY_MAPPER_TEST(TRAITS, PREFIX, PROPERTY_VALUE_TRAITS, CanMapPropertyTransactionWithoutModifications) \
	MAKE_PROPERTY_MAPPER_TEST(TRAITS, PREFIX, PROPERTY_VALUE_TRAITS, CanMapPropertyTransactionWithSingleModification) \
	MAKE_PROPERTY_MAPPER_TEST(TRAITS, PREFIX, PROPERTY_VALUE_TRAITS, CanMapPropertyTransactionWithMultipleModifications)

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT_WITH_PREFIXED_TRAITS(AddressProperty, Address)
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT_WITH_PREFIXED_TRAITS(MosaicProperty, Mosaic)
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT_WITH_PREFIXED_TRAITS(TransactionTypeProperty, TransactionType)
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_WITH_PREFIXED_TRAITS(
			TEST_CLASS,
			Address,
			_Address,
			model::Entity_Type_Address_Property)
	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_WITH_PREFIXED_TRAITS(
			TEST_CLASS,
			Mosaic,
			_Mosaic,
			model::Entity_Type_Mosaic_Property)
	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_WITH_PREFIXED_TRAITS(
			TEST_CLASS,
			TransactionType,
			_TransactionType,
			model::Entity_Type_Transaction_Type_Property)

	DEFINE_PROPERTY_MAPPER_TESTS_WITH_PREFIXED_TRAITS(Traits, Address, test::BaseAddressPropertyTraits)
	DEFINE_PROPERTY_MAPPER_TESTS_WITH_PREFIXED_TRAITS(Traits, Mosaic, test::BaseMosaicPropertyTraits)
	DEFINE_PROPERTY_MAPPER_TESTS_WITH_PREFIXED_TRAITS(Traits, TransactionType, test::BaseTransactionTypePropertyTraits)
}}}
