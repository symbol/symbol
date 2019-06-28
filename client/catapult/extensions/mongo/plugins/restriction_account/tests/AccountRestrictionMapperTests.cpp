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
#include "mongo/tests/test/MongoTransactionPluginTestUtils.h"
#include "tests/test/AccountRestrictionCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS AccountRestrictionMapperTests

	template<typename TTraits, typename TRestrictionValueTraits>
	class AccountRestrictionMapperTests {
	public:
		static void AssertCanMapAccountRestrictionTransactionWithoutModifications() {
			// Assert:
			AssertCanMapAccountRestrictionTransaction(0);
		}

		static void AssertCanMapAccountRestrictionTransactionWithSingleModification() {
			// Assert:
			AssertCanMapAccountRestrictionTransaction(1);
		}

		static void AssertCanMapAccountRestrictionTransactionWithMultipleModifications() {
			// Assert:
			AssertCanMapAccountRestrictionTransaction(5);
		}

	private:
		static auto CreateAccountRestrictionTransaction(uint8_t numModifications) {
			using TransactionType = typename TTraits::TransactionType;
			auto valueSize = TRestrictionValueTraits::Restriction_Value_Size;
			auto entitySize = static_cast<uint32_t>(sizeof(TransactionType) + numModifications * (1 + valueSize));
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });
			pTransaction->Size = entitySize;
			pTransaction->ModificationsCount = numModifications;
			return pTransaction;
		}

		template<typename TTransaction>
		static void AssertAccountRestrictionTransaction(const TTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
			EXPECT_EQ(
					transaction.RestrictionType,
					static_cast<model::AccountRestrictionType>(test::GetUint8(dbTransaction, "restrictionType")));

			auto dbModifications = dbTransaction["modifications"].get_array().value;
			ASSERT_EQ(transaction.ModificationsCount, test::GetFieldCount(dbModifications));
			const auto* pModification = transaction.ModificationsPtr();
			auto iter = dbModifications.cbegin();
			for (auto i = 0u; i < transaction.ModificationsCount; ++i) {
				auto view = iter->get_document().view();
				EXPECT_EQ(
						pModification->ModificationType,
						static_cast<model::AccountRestrictionModificationType>(test::GetUint8(view, "type")));

				RawBuffer buffer(view["value"].get_binary().bytes, view["value"].get_binary().size);
				EXPECT_EQ(pModification->Value, TRestrictionValueTraits::FromBuffer(buffer));
				++pModification;
				++iter;
			}
		}

		static void AssertCanMapAccountRestrictionTransaction(uint8_t numModifications) {
			// Arrange:
			auto pTransaction = CreateAccountRestrictionTransaction(numModifications);
			auto pPlugin = TTraits::CreatePlugin();

			// Act:
			mappers::bson_stream::document builder;
			pPlugin->streamTransaction(builder, *pTransaction);
			auto view = builder.view();

			// Assert:
			EXPECT_EQ(2u, test::GetFieldCount(view));
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
	MAKE_ACCOUNT_RESTRICTION_MAPPER_TEST(TRAITS, PREFIX, RESTRICTION_VALUE_TRAITS, WithSingleModification) \
	MAKE_ACCOUNT_RESTRICTION_MAPPER_TEST(TRAITS, PREFIX, RESTRICTION_VALUE_TRAITS, WithMultipleModifications)

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
