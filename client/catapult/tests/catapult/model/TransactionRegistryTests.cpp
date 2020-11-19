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
		registry.registerPlugin(mocks::CreateMockTransactionPlugin(static_cast<model::EntityType>(124)));

		// Assert:
		EXPECT_EQ(1u, registry.size());
	}

	TEST(TEST_CLASS, CanRegisterMultiplePlugin) {
		// Act:
		TransactionRegistry registry;
		for (auto i : { 123, 7, 222 })
			registry.registerPlugin(mocks::CreateMockTransactionPlugin(static_cast<model::EntityType>(i)));

		// Assert:
		EXPECT_EQ(3u, registry.size());
	}

	TEST(TEST_CLASS, CannotRegisterMultiplePluginsWithSameType) {
		// Arrange:
		TransactionRegistry registry;
		registry.registerPlugin(mocks::CreateMockTransactionPlugin(static_cast<model::EntityType>(124)));

		// Act + Assert:
		EXPECT_THROW(
				registry.registerPlugin(mocks::CreateMockTransactionPlugin(static_cast<model::EntityType>(124))),
				catapult_invalid_argument);
	}

	// endregion

	// region findPlugin

	TEST(TEST_CLASS, CanFindRegistedPlugin) {
		// Arrange:
		TransactionRegistry registry;
		for (auto i : { 123, 7, 222 })
			registry.registerPlugin(mocks::CreateMockTransactionPlugin(static_cast<model::EntityType>(i)));

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
			registry.registerPlugin(mocks::CreateMockTransactionPlugin(static_cast<model::EntityType>(i)));

		// Act:
		const auto* pPlugin = registry.findPlugin(static_cast<EntityType>(8));

		// Assert:
		EXPECT_FALSE(!!pPlugin);
	}

	// endregion
}}
