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

#include "catapult/net/PeerConnectResult.h"
#include "tests/TestHarness.h"

namespace catapult { namespace net {

#define TEST_CLASS PeerConnectResultTests

	TEST(TEST_CLASS, CanCreateDefaultResult) {
		// Act:
		auto result = PeerConnectResult();

		// Assert:
		EXPECT_EQ(static_cast<PeerConnectCode>(-1), result.Code);
		EXPECT_EQ(Key(), result.IdentityKey);
	}

	TEST(TEST_CLASS, CanCreateDefaultResultFromCode) {
		// Act:
		auto result = PeerConnectResult(PeerConnectCode::Accepted);

		// Assert:
		EXPECT_EQ(PeerConnectCode::Accepted, result.Code);
		EXPECT_EQ(Key(), result.IdentityKey);
	}

	TEST(TEST_CLASS, CanCreateDefaultResultFromCodeAndKey_Success) {
		// Act:
		auto key = test::GenerateRandomByteArray<Key>();
		auto result = PeerConnectResult(PeerConnectCode::Accepted, key);

		// Assert:
		EXPECT_EQ(PeerConnectCode::Accepted, result.Code);
		EXPECT_EQ(key, result.IdentityKey);
	}

	TEST(TEST_CLASS, CanCreateDefaultResultFromCodeAndKey_Error) {
		// Act:
		auto key = test::GenerateRandomByteArray<Key>();
		auto result = PeerConnectResult(PeerConnectCode::Already_Connected, key);

		// Assert:
		EXPECT_EQ(PeerConnectCode::Already_Connected, result.Code);
		EXPECT_EQ(Key(), result.IdentityKey);
	}
}}
