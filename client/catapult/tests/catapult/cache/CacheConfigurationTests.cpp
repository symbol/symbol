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

#include "catapult/cache/CacheConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS CacheConfigurationTests

	TEST(TEST_CLASS, CanCreateDefaultConfiguration) {
		// Act:
		CacheConfiguration config;

		// Assert:
		EXPECT_FALSE(config.ShouldUseCacheDatabase);
		EXPECT_TRUE(config.CacheDatabaseDirectory.empty());
		EXPECT_EQ(utils::FileSize(), config.CacheDatabaseConfig.MaxWriteBatchSize);
		EXPECT_FALSE(config.ShouldStorePatriciaTrees);
	}

	TEST(TEST_CLASS, CanCreateConfigurationWithPathButNotPatriciaTreeStorage_DefaultConfig) {
		// Act:
		CacheConfiguration config("xyz", PatriciaTreeStorageMode::Disabled);

		// Assert:
		EXPECT_TRUE(config.ShouldUseCacheDatabase);
		EXPECT_EQ("xyz", config.CacheDatabaseDirectory);
		EXPECT_EQ(utils::FileSize::FromMegabytes(0), config.CacheDatabaseConfig.MaxWriteBatchSize);
		EXPECT_FALSE(config.ShouldStorePatriciaTrees);
	}

	TEST(TEST_CLASS, CanCreateConfigurationWithPathButNotPatriciaTreeStorage_CustomConfig) {
		// Act:
		auto cacheDatabaseConfig = config::NodeConfiguration::CacheDatabaseSubConfiguration();
		cacheDatabaseConfig.MaxWriteBatchSize = utils::FileSize::FromMegabytes(4);
		CacheConfiguration config("xyz", cacheDatabaseConfig, PatriciaTreeStorageMode::Disabled);

		// Assert:
		EXPECT_TRUE(config.ShouldUseCacheDatabase);
		EXPECT_EQ("xyz", config.CacheDatabaseDirectory);
		EXPECT_EQ(utils::FileSize::FromMegabytes(4), config.CacheDatabaseConfig.MaxWriteBatchSize);
		EXPECT_FALSE(config.ShouldStorePatriciaTrees);
	}

	TEST(TEST_CLASS, CanCreateConfigurationWithPathAndPatriciaTreeStorage_DefaultConfig) {
		// Act:
		CacheConfiguration config("xyz", PatriciaTreeStorageMode::Enabled);

		// Assert:
		EXPECT_TRUE(config.ShouldUseCacheDatabase);
		EXPECT_EQ("xyz", config.CacheDatabaseDirectory);
		EXPECT_EQ(utils::FileSize::FromMegabytes(0), config.CacheDatabaseConfig.MaxWriteBatchSize);
		EXPECT_TRUE(config.ShouldStorePatriciaTrees);
	}

	TEST(TEST_CLASS, CanCreateConfigurationWithPathAndPatriciaTreeStorage_CustomConfig) {
		// Act:
		auto cacheDatabaseConfig = config::NodeConfiguration::CacheDatabaseSubConfiguration();
		cacheDatabaseConfig.MaxWriteBatchSize = utils::FileSize::FromMegabytes(4);
		CacheConfiguration config("xyz", cacheDatabaseConfig, PatriciaTreeStorageMode::Enabled);

		// Assert:
		EXPECT_TRUE(config.ShouldUseCacheDatabase);
		EXPECT_EQ("xyz", config.CacheDatabaseDirectory);
		EXPECT_EQ(utils::FileSize::FromMegabytes(4), config.CacheDatabaseConfig.MaxWriteBatchSize);
		EXPECT_TRUE(config.ShouldStorePatriciaTrees);
	}
}}
