#include "catapult/model/TransactionRegistry.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	// region basic

	TEST(TransactionRegistryTests, NoPluginsAreInitiallyRegistered) {
		// Act:
		TransactionRegistry registry;

		// Assert:
		EXPECT_EQ(0u, registry.size());
	}

	// endregion

	// region registerPlugin

	TEST(TransactionRegistryTests, CanRegisterPlugin) {
		// Act:
		TransactionRegistry registry;
		registry.registerPlugin(mocks::CreateMockTransactionPlugin(124));

		// Assert:
		EXPECT_EQ(1u, registry.size());
	}

	TEST(TransactionRegistryTests, CanRegisterMultiplePlugin) {
		// Act:
		TransactionRegistry registry;
		for (auto i : { 123, 7, 222 })
			registry.registerPlugin(mocks::CreateMockTransactionPlugin(i));

		// Assert:
		EXPECT_EQ(3u, registry.size());
	}

	TEST(TransactionRegistryTests, CannotRegisterMultiplePluginsWithSameType) {
		// Act:
		TransactionRegistry registry;
		registry.registerPlugin(mocks::CreateMockTransactionPlugin(124));

		// Assert:
		EXPECT_THROW(registry.registerPlugin(mocks::CreateMockTransactionPlugin(124)), catapult_invalid_argument);
	}

	// endregion

	// region findPlugin

	TEST(TransactionRegistryTests, CanFindRegistedPlugin) {
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

	TEST(TransactionRegistryTests, CannotFindUnregistedPlugin) {
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
