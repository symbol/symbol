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

#include "catapult/cache/CacheDatabaseMixin.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS CacheDatabaseMixinTests

	namespace {
		class ConcreteCacheDatabaseMixin : public CacheDatabaseMixin {
		public:
			ConcreteCacheDatabaseMixin(
					const CacheConfiguration& config,
					const std::vector<std::string>& columnFamilyNames,
					FilterPruningMode pruningMode = FilterPruningMode::Disabled)
					: CacheDatabaseMixin(config, columnFamilyNames, pruningMode)
			{}

		public:
			bool hasPatriciaTreeSupport() const {
				return CacheDatabaseMixin::hasPatriciaTreeSupport();
			}

			CacheDatabase& database() {
				return CacheDatabaseMixin::database();
			}

			void flush() {
				return CacheDatabaseMixin::flush();
			}

		public:
			static deltaset::ConditionalContainerMode GetContainerMode(const CacheConfiguration& config) {
				return CacheDatabaseMixin::GetContainerMode(config);
			}
		};
	}

	TEST(TEST_CLASS, DatabaseInitializationIsBypassedWhenCacheDatabaseIsDisabled) {
		// Arrange:
		CacheConfiguration config;

		// Act:
		ConcreteCacheDatabaseMixin mixin(config, { "default", "foo", "bar" });

		// Assert:
		EXPECT_FALSE(mixin.hasPatriciaTreeSupport());

		EXPECT_TRUE(mixin.database().columnFamilyNames().empty());
		EXPECT_FALSE(mixin.database().canPrune());

		EXPECT_EQ(deltaset::ConditionalContainerMode::Memory, decltype(mixin)::GetContainerMode(config));
	}

	TEST(TEST_CLASS, CanInitializeBasicDatabase) {
		// Arrange:
		test::TempDirectoryGuard dbDirGuard;
		CacheConfiguration config(dbDirGuard.name(), PatriciaTreeStorageMode::Disabled);

		// Act:
		ConcreteCacheDatabaseMixin mixin(config, { "default", "foo", "bar" });

		// Assert:
		EXPECT_FALSE(mixin.hasPatriciaTreeSupport());

		EXPECT_EQ((std::vector<std::string>{ "default", "foo", "bar" }), mixin.database().columnFamilyNames());
		EXPECT_FALSE(mixin.database().canPrune());

		EXPECT_EQ(deltaset::ConditionalContainerMode::Storage, decltype(mixin)::GetContainerMode(config));
	}

	TEST(TEST_CLASS, CanInitializeDatabaseWithPruning) {
		// Arrange:
		test::TempDirectoryGuard dbDirGuard;
		CacheConfiguration config(dbDirGuard.name(), PatriciaTreeStorageMode::Disabled);

		// Act:
		ConcreteCacheDatabaseMixin mixin(config, { "default", "foo", "bar" }, FilterPruningMode::Enabled);

		// Assert:
		EXPECT_FALSE(mixin.hasPatriciaTreeSupport());

		EXPECT_EQ((std::vector<std::string>{ "default", "foo", "bar" }), mixin.database().columnFamilyNames());
		EXPECT_TRUE(mixin.database().canPrune());

		EXPECT_EQ(deltaset::ConditionalContainerMode::Storage, decltype(mixin)::GetContainerMode(config));
	}

	TEST(TEST_CLASS, CanInitializeDatabaseWithPatriciaTreeSupport) {
		// Arrange:
		test::TempDirectoryGuard dbDirGuard;
		CacheConfiguration config(dbDirGuard.name(), PatriciaTreeStorageMode::Enabled);

		// Act:
		ConcreteCacheDatabaseMixin mixin(config, { "default", "foo", "bar" });

		// Assert:
		EXPECT_TRUE(mixin.hasPatriciaTreeSupport());

		EXPECT_EQ((std::vector<std::string>{ "default", "foo", "bar", "patricia_tree" }), mixin.database().columnFamilyNames());
		EXPECT_FALSE(mixin.database().canPrune());

		EXPECT_EQ(deltaset::ConditionalContainerMode::Storage, decltype(mixin)::GetContainerMode(config));
	}

	TEST(TEST_CLASS, CanFlushWhenCacheDatabaseIsDisabled) {
		// Arrange:
		CacheConfiguration config;
		ConcreteCacheDatabaseMixin mixin(config, { "default", "foo", "bar" });

		// Act + Assert:
		EXPECT_NO_THROW(mixin.flush());
	}

	TEST(TEST_CLASS, CanFlushWhenCacheDatabaseIsEnabled) {
		// Arrange: create mixin with non-zero batch-size
		auto cacheDatabaseConfig = config::NodeConfiguration::CacheDatabaseSubConfiguration();
		cacheDatabaseConfig.MaxWriteBatchSize = utils::FileSize::FromKilobytes(100);

		test::TempDirectoryGuard dbDirGuard;
		CacheConfiguration config(dbDirGuard.name(), cacheDatabaseConfig, PatriciaTreeStorageMode::Disabled);
		ConcreteCacheDatabaseMixin mixin(config, { "default", "foo", "bar" });

		// Act + Assert:
		EXPECT_NO_THROW(mixin.flush());
	}
}}
