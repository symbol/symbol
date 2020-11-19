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

#include "mongo/src/MongoTransactionPluginFactory.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo {

#define TEST_CLASS MongoTransactionPluginFactoryTests

	namespace {
		constexpr auto Mock_Transaction_Type = static_cast<model::EntityType>(0x4FFF);

		template<typename TTransaction>
		void Stream(bsoncxx::builder::stream::document& builder, const TTransaction& transaction) {
			builder << "signerPublicKey0" << transaction.SignerPublicKey[0];
		}

		struct RegularTraits {
			using TransactionType = mocks::MockTransaction;

			static auto CreatePlugin() {
				return MongoTransactionPluginFactory::Create<mocks::MockTransaction, mocks::EmbeddedMockTransaction>(
						Stream<mocks::MockTransaction>,
						Stream<mocks::EmbeddedMockTransaction>);
			}
		};

		struct EmbeddedTraits {
			using TransactionType = mocks::EmbeddedMockTransaction;

			static auto CreatePlugin() {
				return MongoTransactionPluginFactory::CreateEmbedded<mocks::EmbeddedMockTransaction>(
						Stream<mocks::EmbeddedMockTransaction>);
			}
		};

#define PLUGIN_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()
	}

	TEST(TEST_CLASS, CanCreateTransactionPluginWithEmbeddingSupport) {
		// Act:
		auto pPlugin = RegularTraits::CreatePlugin();

		// Assert:
		ASSERT_TRUE(pPlugin->supportsEmbedding());
		EXPECT_EQ(Mock_Transaction_Type, pPlugin->embeddedPlugin().type());
	}

	TEST(TEST_CLASS, DependentDocumentsAreNotSupportedByDefault) {
		// Arrange:
		auto pPlugin = RegularTraits::CreatePlugin();

		RegularTraits::TransactionType transaction;
		auto transactionInfo = test::CreateRandomTransactionInfo();
		auto metadata = MongoTransactionMetadata(transactionInfo);

		// Act:
		auto documents = pPlugin->extractDependentDocuments(transaction, metadata);

		// Assert:
		EXPECT_TRUE(documents.empty());
	}

	PLUGIN_TEST(CanCreatePlugin) {
		// Act:
		auto pPlugin = TTraits::CreatePlugin();

		// Assert:
		EXPECT_EQ(Mock_Transaction_Type, pPlugin->type());
	}

	PLUGIN_TEST(CanStreamTransaction) {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin();
		bsoncxx::builder::stream::document builder;

		typename TTraits::TransactionType transaction;
		transaction.SignerPublicKey[0] = 0x57;

		// Act:
		pPlugin->streamTransaction(builder, transaction);
		auto dbTransaction = builder << bsoncxx::builder::stream::finalize;

		// Assert:
		auto view = dbTransaction.view();
		EXPECT_EQ(1u, test::GetFieldCount(view));
		EXPECT_EQ(0x57u, test::GetUint32(view, "signerPublicKey0"));
	}
}}
