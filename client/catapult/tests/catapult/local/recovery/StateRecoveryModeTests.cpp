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

#include "catapult/local/recovery/StateRecoveryMode.h"
#include "catapult/config/NodeConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS StateRecoveryModeTests

	namespace {
		StateRecoveryMode CalculateStateRecoveryMode(
				Height::ValueType cacheHeight,
				Height::ValueType storageHeight,
				bool enableCacheDatabaseStorage) {
			// Arrange:
			auto config = config::NodeConfiguration::Uninitialized();
			config.EnableCacheDatabaseStorage = enableCacheDatabaseStorage;

			// Act:
			return local::CalculateStateRecoveryMode(config, { Height(cacheHeight), Height(storageHeight) });
		}
	}

	TEST(TEST_CLASS, NoneWhenCacheAndStorageHeightsAreEqual) {
		EXPECT_EQ(StateRecoveryMode::None, CalculateStateRecoveryMode(1, 1, false));
		EXPECT_EQ(StateRecoveryMode::None, CalculateStateRecoveryMode(1, 1, true));

		EXPECT_EQ(StateRecoveryMode::None, CalculateStateRecoveryMode(10, 10, false));
		EXPECT_EQ(StateRecoveryMode::None, CalculateStateRecoveryMode(10, 10, true));
	}

	TEST(TEST_CLASS, ReseedWhenCacheHeightIsOneAndStorageHeightIsGreaterThanOne) {
		EXPECT_EQ(StateRecoveryMode::Reseed, CalculateStateRecoveryMode(1, 10, false));
		EXPECT_EQ(StateRecoveryMode::Reseed, CalculateStateRecoveryMode(1, 10, true));
	}

	TEST(TEST_CLASS, ExceptionWhenCacheHeightIsGreaterThanStorageHeight) {
		EXPECT_THROW(CalculateStateRecoveryMode(10, 9, false), catapult_invalid_argument);
		EXPECT_THROW(CalculateStateRecoveryMode(10, 9, true), catapult_invalid_argument);

		EXPECT_THROW(CalculateStateRecoveryMode(10, 1, false), catapult_invalid_argument);
		EXPECT_THROW(CalculateStateRecoveryMode(10, 1, true), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, RepairWhenCacheHeightIsGreaterThanOneAndLessThanStorageHeight_CacheDatabaseDisabled) {
		EXPECT_EQ(StateRecoveryMode::Repair, CalculateStateRecoveryMode(2, 10, false));
		EXPECT_EQ(StateRecoveryMode::Repair, CalculateStateRecoveryMode(9, 10, false));
	}

	TEST(TEST_CLASS, NoneWhenCacheHeightIsGreaterThanOneAndLessThanStorageHeight_CacheDatabaseEnabled) {
		EXPECT_EQ(StateRecoveryMode::None, CalculateStateRecoveryMode(2, 10, true));
		EXPECT_EQ(StateRecoveryMode::None, CalculateStateRecoveryMode(9, 10, true));
	}
}}
