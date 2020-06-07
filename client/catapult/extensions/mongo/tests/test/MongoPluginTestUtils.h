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
#include "mongo/src/MongoPluginManager.h"
#include "mongo/src/MongoStorageContext.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "tests/TestHarness.h"
#include <mongocxx/instance.hpp>

namespace catapult { namespace test {

	template<typename TAction>
	void RunTestAfterRegistration(decltype(::RegisterMongoSubsystem)* registerSubsystem, TAction action) {
		// Arrange:
		// - windows requires the caller to explicitly create a mongocxx instance before certain operations
		//   like creating a mongocxx::pool (via MongoStorageContext)
		mongocxx::instance::current();
		auto pPool = CreateStartedIoThreadPool(Num_Default_Mongo_Test_Pool_Threads);
		auto pMongoContext = CreateDefaultMongoStorageContext(DatabaseName(), *pPool);
		mongo::MongoPluginManager manager(*pMongoContext, model::NetworkIdentifier::Zero);
		registerSubsystem(manager);

		// Act:
		action(manager);
	}

	/// Asserts that transactions have been registered.
	template<typename TTraits>
	void AssertAppropriateTransactionsAreRegistered() {
		// Arrange:
		RunTestAfterRegistration(TTraits::RegisterSubsystem, [](const auto& manager) {
			// Act:
			const auto& transactionRegistry = manager.transactionRegistry();

			// Assert:
			auto expectedTypes = TTraits::GetTransactionTypes();
			EXPECT_EQ(expectedTypes.size(), transactionRegistry.size());

			for (const auto type : expectedTypes) {
				CATAPULT_LOG(debug) << "checking type " << type;
				EXPECT_TRUE(!!transactionRegistry.findPlugin(type));
			}
		});
	}

	/// Asserts that receipts have been registered.
	template<typename TTraits>
	void AssertAppropriateReceiptsAreRegistered() {
		// Arrange:
		RunTestAfterRegistration(TTraits::RegisterSubsystem, [](const auto& manager) {
			// Act:
			const auto& receiptRegistry = manager.receiptRegistry();

			// Assert:
			auto expectedTypes = TTraits::GetReceiptTypes();
			EXPECT_EQ(expectedTypes.size(), receiptRegistry.size());

			for (const auto type : expectedTypes) {
				CATAPULT_LOG(debug) << "checking type " << type;
				EXPECT_TRUE(!!receiptRegistry.findPlugin(type));
			}
		});
	}

	/// Asserts that storages have been registered.
	template<typename TTraits>
	void AssertAppropriateStoragesAreRegistered() {
		// Arrange:
		RunTestAfterRegistration(TTraits::RegisterSubsystem, [](auto& manager) {
			// Act:
			auto pStorage = manager.createStorage();

			// Assert:
			EXPECT_EQ(TTraits::GetStorageName(), pStorage->name());
		});
	}

#define MAKE_MONGO_PLUGIN_TEST(TEST_CLASS, TEST_TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::Assert##TEST_NAME<TEST_TRAITS>(); }

#define DEFINE_MONGO_PLUGIN_TESTS(TEST_CLASS, TEST_TRAITS) \
	MAKE_MONGO_PLUGIN_TEST(TEST_CLASS, TEST_TRAITS, AppropriateTransactionsAreRegistered) \
	MAKE_MONGO_PLUGIN_TEST(TEST_CLASS, TEST_TRAITS, AppropriateReceiptsAreRegistered) \
	MAKE_MONGO_PLUGIN_TEST(TEST_CLASS, TEST_TRAITS, AppropriateStoragesAreRegistered)
}}
