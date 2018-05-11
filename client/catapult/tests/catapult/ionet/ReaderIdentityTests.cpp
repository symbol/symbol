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

#include "catapult/ionet/ReaderIdentity.h"
#include "catapult/crypto/KeyUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

	TEST(TEST_CLASS, CanOutputReaderIdentity) {
		// Arrange:
		auto key = crypto::ParseKey("1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751");
		auto identity = ReaderIdentity{ key, "bob.com" };

		// Act:
		auto str = test::ToString(identity);

		// Assert:
		EXPECT_EQ("reader (1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751 @ bob.com)", str);
	}
}}
