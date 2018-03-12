#pragma once
#include "mongo/src/MongoPluginManager.h"
#include "mongo/src/MongoStorageContext.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "tests/TestHarness.h"
#include <mongocxx/instance.hpp>

namespace catapult { namespace test {

	using RegisterMongoSubsystem = consumer<mongo::MongoPluginManager&>;

	template<typename TAction>
	void RunTestAfterRegistration(const RegisterMongoSubsystem& registerSubsystem, TAction action) {
		// Arrange:
		// - windows requires the caller to explicitly create a mongocxx instance before certain operations
		//   like creating a mongocxx::pool (via MongoStorageContext)
		mongocxx::instance::current();
		mongo::MongoStorageContext mongoContext(test::DefaultDbUri(), "", nullptr);
		auto chainConfig = model::BlockChainConfiguration::Uninitialized();
		mongo::MongoPluginManager manager(mongoContext, chainConfig);
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
	MAKE_MONGO_PLUGIN_TEST(TEST_CLASS, TEST_TRAITS, AppropriateStoragesAreRegistered)
}}
