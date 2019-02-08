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

#include "catapult/extensions/ConfigurationUtils.h"
#include "catapult/config/NodeConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS ConfigurationUtilsTests

	TEST(TEST_CLASS, CanExtractUtCacheOptionsFromNodeConfiguration) {
		// Arrange:
		auto config = config::NodeConfiguration::Uninitialized();
		config.UnconfirmedTransactionsCacheMaxResponseSize = utils::FileSize::FromKilobytes(4);
		config.UnconfirmedTransactionsCacheMaxSize = 234;

		// Act:
		auto options = GetUtCacheOptions(config);

		// Assert:
		EXPECT_EQ(4096u, options.MaxResponseSize);
		EXPECT_EQ(234u, options.MaxCacheSize);
	}
}}
