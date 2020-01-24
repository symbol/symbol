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

#include "harvesting/src/UnlockedEntryMessage.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS UnlockedEntryMessageTests

	TEST(TEST_CLASS, CanGetEncryptedUnlockedEntrySize) {
		EXPECT_EQ(32u + 16 + 32 + 16, EncryptedUnlockedEntrySize());
	}

	TEST(TEST_CLASS, CanGetMessageIdentifierFromMessage) {
		// Arrange:
		auto expectedMessageIdentifier = test::GenerateRandomByteArray<UnlockedEntryMessageIdentifier>();

		UnlockedEntryMessage message;
		message.EncryptedEntry = RawBuffer{ expectedMessageIdentifier.data(), expectedMessageIdentifier.size() };

		// Act:
		auto messageIdentifier = GetMessageIdentifier(message);

		// Assert:
		EXPECT_EQ(expectedMessageIdentifier, messageIdentifier);
	}
}}
