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

#include "catapult/extensions/PluginUtils.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/validators/AggregateEntityValidator.h"
#include "tests/test/local/LocalTestUtils.h"

namespace catapult { namespace extensions {

#define TEST_CLASS PluginUtilsTests

	TEST(TEST_CLASS, CanCreateStorageConfiguration) {
		// Arrange:
		auto nodeConfig = config::NodeConfiguration::Uninitialized();
		nodeConfig.ShouldUseCacheDatabaseStorage = true;
		nodeConfig.MaxCacheDatabaseWriteBatchSize = utils::FileSize::FromKilobytes(123);

		auto userConfig = config::UserConfiguration::Uninitialized();
		userConfig.DataDirectory = "foo_bar";

		auto config = config::LocalNodeConfiguration(
				model::BlockChainConfiguration::Uninitialized(),
				std::move(nodeConfig),
				config::LoggingConfiguration::Uninitialized(),
				std::move(userConfig));

		// Act:
		auto storageConfig = CreateStorageConfiguration(config);

		// Assert:
		EXPECT_TRUE(storageConfig.PreferCacheDatabase);
		EXPECT_EQ("foo_bar/statedb", storageConfig.CacheDatabaseDirectory);
		EXPECT_EQ(utils::FileSize::FromKilobytes(123), storageConfig.MaxCacheDatabaseWriteBatchSize);
	}

	TEST(TEST_CLASS, CanCreateStatelessValidator) {
		// Arrange:
		auto pPluginManager = test::CreateDefaultPluginManager();

		// Act:
		auto pEntityValidator = CreateStatelessValidator(*pPluginManager);

		// Assert:
		ASSERT_TRUE(!!pEntityValidator);

		// - notice that, as an implementation detail, the returned validator is an aggregate of one
		//   (the entity -> notification adapter)
		ASSERT_EQ(1u, pEntityValidator->names().size());
		EXPECT_EQ(pPluginManager->createStatelessValidator()->name(), pEntityValidator->names()[0]);
	}

	TEST(TEST_CLASS, CanCreateEntityObserver) {
		// Arrange:
		auto pPluginManager = test::CreateDefaultPluginManager();

		// Act:
		auto pEntityObserver = CreateEntityObserver(*pPluginManager);

		// Assert:
		ASSERT_TRUE(!!pEntityObserver);
		EXPECT_EQ(pPluginManager->createObserver()->name(), pEntityObserver->name());
	}

	TEST(TEST_CLASS, CanCreateUndoEntityObserver) {
		// Arrange:
		auto pPluginManager = test::CreateDefaultPluginManager();

		// Act:
		auto pEntityObserver = CreateUndoEntityObserver(*pPluginManager);

		// Assert:
		ASSERT_TRUE(!!pEntityObserver);
		EXPECT_EQ(pPluginManager->createObserver()->name(), pEntityObserver->name());
	}

	TEST(TEST_CLASS, CanCreatePermanentEntityObserver) {
		// Arrange:
		auto pPluginManager = test::CreateDefaultPluginManager();

		// Act:
		auto pEntityObserver = CreatePermanentEntityObserver(*pPluginManager);

		// Assert:
		ASSERT_TRUE(!!pEntityObserver);
		EXPECT_EQ(pPluginManager->createPermanentObserver()->name(), pEntityObserver->name());
	}
}}
