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
#include "catapult/config/CatapultConfiguration.h"
#include "catapult/validators/AggregateEntityValidator.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/other/MutableCatapultConfiguration.h"

namespace catapult { namespace extensions {

#define TEST_CLASS PluginUtilsTests

	TEST(TEST_CLASS, CanCreateStorageConfiguration) {
		// Arrange:
		test::MutableCatapultConfiguration config;
		config.Node.ShouldUseCacheDatabaseStorage = true;
		config.Node.MaxCacheDatabaseWriteBatchSize = utils::FileSize::FromKilobytes(123);
		config.User.DataDirectory = "foo_bar";

		// Act:
		auto storageConfig = CreateStorageConfiguration(config.ToConst());

		// Assert:
		EXPECT_TRUE(storageConfig.PreferCacheDatabase);
		EXPECT_EQ("foo_bar/statedb", storageConfig.CacheDatabaseDirectory);
		EXPECT_EQ(utils::FileSize::FromKilobytes(123), storageConfig.MaxCacheDatabaseWriteBatchSize);
	}

	TEST(TEST_CLASS, CanCreateStatelessValidator) {
		// Arrange:
		auto pPluginManager = test::CreateDefaultPluginManagerWithRealPlugins();

		// Act:
		auto pEntityValidator = CreateStatelessValidator(*pPluginManager);

		// Assert:
		ASSERT_TRUE(!!pEntityValidator);

		// - notice that, as an implementation detail, the returned validator is an aggregate of one
		//   (the entity -> notification adapter)
		ASSERT_EQ(1u, pEntityValidator->names().size());
		EXPECT_EQ(pPluginManager->createStatelessValidator()->name(), pEntityValidator->names()[0]);
	}

	TEST(TEST_CLASS, CanCreateUndoEntityObserver) {
		// Arrange:
		auto pPluginManager = test::CreateDefaultPluginManagerWithRealPlugins();

		// Act:
		auto pEntityObserver = CreateUndoEntityObserver(*pPluginManager);

		// Assert:
		ASSERT_TRUE(!!pEntityObserver);
		EXPECT_EQ(pPluginManager->createObserver()->name(), pEntityObserver->name());
	}
}}
