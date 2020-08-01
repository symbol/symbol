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

#include "src/AccountRestrictionMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/restriction_account/src/model/AccountAddressRestrictionTransaction.h"
#include "plugins/txes/restriction_account/src/model/AccountMosaicRestrictionTransaction.h"
#include "plugins/txes/restriction_account/src/model/AccountOperationRestrictionTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTests.h"
#include "tests/test/AccountRestrictionCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS AccountRestrictionMapperTests

	template<typename TTraits, typename TRestrictionValueTraits>
	class AccountRestrictionMapperTests {
	public:
		static void AssertCanMapAccountRestrictionTransactionWithoutModifications() {
			AssertCanMapAccountRestrictionTransaction(0, 0);
		}

		static void AssertCanMapAccountRestrictionTransactionWithSingleAddition() {
			AssertCanMapAccountRestrictionTransaction(1, 0);
		}

		static void AssertCanMapAccountRestrictionTransactionWithSingleDeletion() {
			AssertCanMapAccountRestrictionTransaction(0, 1);
		}

		static void AssertCanMapAccountRestrictionTransactionWithMultipleAdditionsAndDeletions() {
			AssertCanMapAccountRestrictionTransaction(3, 2);
		}

	private:
		static auto CreateAccountRestrictionTransaction(uint8_t numAdditions, uint8_t numDeletions) {
			using TransactionType = typename TTraits::TransactionType;
			auto valueSize = TRestrictionValueTraits::Restriction_Value_Size;
			auto entitySize = SizeOf32<TransactionType>() + (numAdditions + numDeletions) * static_cast<uint32_t>(valueSize);
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });
			pTransaction->Size = entitySize;
			pTransaction->RestrictionAdditionsCount = numAdditions;
			pTransaction->RestrictionDeletionsCount = numDeletions;
			return pTransaction;
		}

		template<typename TRestrictionValue>
		static void AssertEqualValues(
				const bsoncxx::document::view& dbTransaction,
				const std::string& name,
				const TRestrictionValue* pValues,
				uint8_t numValues) {
			auto dbValues = dbTransaction[name].get_array().value;
			ASSERT_EQ(numValues, test::GetFieldCount(dbValues));

			auto dbValuesIter = dbValues.cbegin();
			for (auto i = 0u; i < numValues; ++i, ++dbValuesIter) {
				RawBuffer buffer(dbValuesIter->get_binary().bytes, dbValuesIter->get_binary().size);
				EXPECT_EQ(pValues[i], TRestrictionValueTraits::FromBuffer(buffer)) << name << " at " << i;
			}
		}

		template<typename TTransaction>
		static void AssertAccountRestrictionTransaction(const TTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
			EXPECT_EQ(
					transaction.RestrictionFlags,
					static_cast<model::AccountRestrictionFlags>(test::GetUint32(dbTransaction, "restrictionFlags")));

			AssertEqualValues(
					dbTransaction,
					"restrictionAdditions",
					transaction.RestrictionAdditionsPtr(),
					transaction.RestrictionAdditionsCount);
			AssertEqualValues(
					dbTransaction,
					"restrictionDeletions",
					transaction.RestrictionDeletionsPtr(),
					transaction.RestrictionDeletionsCount);
		}

		static void AssertCanMapAccountRestrictionTransaction(uint8_t numAdditions, uint8_t numDeletions) {
			// Arrange:
			auto pTransaction = CreateAccountRestrictionTransaction(numAdditions, numDeletions);
			auto pPlugin = TTraits::CreatePlugin();

			// Act:
			mappers::bson_stream::document builder;
			pPlugin->streamTransaction(builder, *pTransaction);
			auto view = builder.view();

			// Assert:
			EXPECT_EQ(3u, test::GetFieldCount(view));
			AssertAccountRestrictionTransaction(*pTransaction, view);
		}
	};

// additional template param is needed in order to handle restriction value correctly, so can't use PLUGIN_TEST
#define MAKE_ACCOUNT_RESTRICTION_MAPPER_TEST(TRAITS, PREFIX, VALUE_TRAITS, TEST_POSTFIX) \
	TEST(TEST_CLASS, CanMapAccountRestrictionTransaction##TEST_POSTFIX##_##PREFIX##_##Regular) { \
		AccountRestrictionMapperTests<PREFIX##Regular##TRAITS, VALUE_TRAITS>::AssertCanMapAccountRestrictionTransaction##TEST_POSTFIX(); \
	} \
	TEST(TEST_CLASS, CanMapAccountRestrictionTransaction##TEST_POSTFIX##_##PREFIX##_Embedded) { \
		AccountRestrictionMapperTests<PREFIX##Embedded##TRAITS, VALUE_TRAITS>::AssertCanMapAccountRestrictionTransaction##TEST_POSTFIX(); \
	}

#define DEFINE_ACCOUNT_RESTRICTION_MAPPER_TESTS_WITH_PREFIXED_TRAITS(TRAITS, PREFIX, RESTRICTION_VALUE_TRAITS) \
	MAKE_ACCOUNT_RESTRICTION_MAPPER_TEST(TRAITS, PREFIX, RESTRICTION_VALUE_TRAITS, WithoutModifications) \
	MAKE_ACCOUNT_RESTRICTION_MAPPER_TEST(TRAITS, PREFIX, RESTRICTION_VALUE_TRAITS, WithSingleAddition) \
	MAKE_ACCOUNT_RESTRICTION_MAPPER_TEST(TRAITS, PREFIX, RESTRICTION_VALUE_TRAITS, WithSingleDeletion) \
	MAKE_ACCOUNT_RESTRICTION_MAPPER_TEST(TRAITS, PREFIX, RESTRICTION_VALUE_TRAITS, WithMultipleAdditionsAndDeletions)

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(AccountAddressRestriction, Address)
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(AccountMosaicRestriction, Mosaic)
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(AccountOperationRestriction, Operation)
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Address, _Address, model::Entity_Type_Account_Address_Restriction)
	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Mosaic, _Mosaic, model::Entity_Type_Account_Mosaic_Restriction)
	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(
			TEST_CLASS,
			Operation,
			_Operation,
			model::Entity_Type_Account_Operation_Restriction)

	DEFINE_ACCOUNT_RESTRICTION_MAPPER_TESTS_WITH_PREFIXED_TRAITS(Traits, Address, test::BaseAccountAddressRestrictionTraits)
	DEFINE_ACCOUNT_RESTRICTION_MAPPER_TESTS_WITH_PREFIXED_TRAITS(Traits, Mosaic, test::BaseAccountMosaicRestrictionTraits)
	DEFINE_ACCOUNT_RESTRICTION_MAPPER_TESTS_WITH_PREFIXED_TRAITS(Traits, Operation, test::BaseAccountOperationRestrictionTraits)
}}}
