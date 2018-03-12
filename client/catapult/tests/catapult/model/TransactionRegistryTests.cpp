#include "catapult/model/TransactionRegistry.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS TransactionRegistryTests

	// region basic

	TEST(TEST_CLASS, NoPluginsAreInitiallyRegistered) {
		// Act:
		TransactionRegistry registry;

		// Assert:
		EXPECT_EQ(0u, registry.size());
	}

	// endregion

	// region registerPlugin

	TEST(TEST_CLASS, CanRegisterPlugin) {
		// Act:
		TransactionRegistry registry;
		registry.registerPlugin(mocks::CreateMockTransactionPlugin(124));

		// Assert:
		EXPECT_EQ(1u, registry.size());
	}

	TEST(TEST_CLASS, CanRegisterMultiplePlugin) {
		// Act:
		TransactionRegistry registry;
		for (auto i : { 123, 7, 222 })
			registry.registerPlugin(mocks::CreateMockTransactionPlugin(i));

		// Assert:
		EXPECT_EQ(3u, registry.size());
	}

	TEST(TEST_CLASS, CannotRegisterMultiplePluginsWithSameType) {
		// Arrange:
		TransactionRegistry registry;
		registry.registerPlugin(mocks::CreateMockTransactionPlugin(124));

		// Act + Assert:
		EXPECT_THROW(registry.registerPlugin(mocks::CreateMockTransactionPlugin(124)), catapult_invalid_argument);
	}

	// endregion

	// region findPlugin

	TEST(TEST_CLASS, CanFindRegistedPlugin) {
		// Arrange:
		TransactionRegistry registry;
		for (auto i : { 123, 7, 222 })
			registry.registerPlugin(mocks::CreateMockTransactionPlugin(i));

		// Act:
		const auto* pPlugin = registry.findPlugin(static_cast<EntityType>(7));

		// Assert:
		ASSERT_TRUE(!!pPlugin);
		EXPECT_EQ(static_cast<EntityType>(7), pPlugin->type());
	}

	TEST(TEST_CLASS, CannotFindUnregistedPlugin) {
		// Arrange:
		TransactionRegistry registry;
		for (auto i : { 123, 7, 222 })
			registry.registerPlugin(mocks::CreateMockTransactionPlugin(i));

		// Act:
		const auto* pPlugin = registry.findPlugin(static_cast<EntityType>(8));

		// Assert:
		EXPECT_FALSE(!!pPlugin);
	}

	// endregion
}}
