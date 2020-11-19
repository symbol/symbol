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

#include "catapult/net/PeerConnectResult.h"
#include "tests/test/core/mocks/MockPacketSocket.h"
#include "tests/TestHarness.h"

namespace catapult { namespace net {

#define TEST_CLASS PeerConnectResultTests

	// region PeerConnectResult

	TEST(TEST_CLASS, CanCreateDefaultResult) {
		// Act:
		auto result = PeerConnectResult();

		// Assert:
		EXPECT_EQ(static_cast<PeerConnectCode>(-1), result.Code);
		EXPECT_EQ(Key(), result.Identity.PublicKey);
		EXPECT_EQ("", result.Identity.Host);
	}

	TEST(TEST_CLASS, CanCreateDefaultResultFromCode) {
		// Act:
		auto result = PeerConnectResult(PeerConnectCode::Accepted);

		// Assert:
		EXPECT_EQ(PeerConnectCode::Accepted, result.Code);
		EXPECT_EQ(Key(), result.Identity.PublicKey);
		EXPECT_EQ("", result.Identity.Host);
	}

	TEST(TEST_CLASS, CanCreateDefaultResultFromCodeAndKey_Success) {
		// Act:
		auto key = test::GenerateRandomByteArray<Key>();
		auto result = PeerConnectResult(PeerConnectCode::Accepted, { key, "11.22.33.44" });

		// Assert:
		EXPECT_EQ(PeerConnectCode::Accepted, result.Code);
		EXPECT_EQ(key, result.Identity.PublicKey);
		EXPECT_EQ("11.22.33.44", result.Identity.Host);
	}

	TEST(TEST_CLASS, CanCreateDefaultResultFromCodeAndKey_Error) {
		// Act:
		auto key = test::GenerateRandomByteArray<Key>();
		auto result = PeerConnectResult(PeerConnectCode::Already_Connected, { key, "11.22.33.44" });

		// Assert:
		EXPECT_EQ(PeerConnectCode::Already_Connected, result.Code);
		EXPECT_EQ(Key(), result.Identity.PublicKey);
		EXPECT_EQ("", result.Identity.Host);
	}

	// endregion

	// region PeerConnectResultEx

	TEST(TEST_CLASS, Ex_CanCreateDefaultResult) {
		// Act:
		auto result = PeerConnectResultEx();

		// Assert:
		EXPECT_EQ(static_cast<PeerConnectCode>(-1), result.Code);
		EXPECT_EQ(Key(), result.Identity.PublicKey);
		EXPECT_EQ("", result.Identity.Host);
		EXPECT_FALSE(!!result.pPeerSocket);
	}

	TEST(TEST_CLASS, Ex_CanCreateDefaultResultFromCode) {
		// Act:
		auto result = PeerConnectResultEx(PeerConnectCode::Accepted);

		// Assert:
		EXPECT_EQ(PeerConnectCode::Accepted, result.Code);
		EXPECT_EQ(Key(), result.Identity.PublicKey);
		EXPECT_EQ("", result.Identity.Host);
		EXPECT_FALSE(!!result.pPeerSocket);
	}

	TEST(TEST_CLASS, Ex_CanCreateDefaultResultFromCodeAndKeyAndSocket_Success) {
		// Act:
		auto key = test::GenerateRandomByteArray<Key>();
		auto pSocket = std::make_shared<mocks::MockPacketSocket>();
		auto result = PeerConnectResultEx(PeerConnectCode::Accepted, { key, "11.22.33.44" }, pSocket);

		// Assert:
		EXPECT_EQ(PeerConnectCode::Accepted, result.Code);
		EXPECT_EQ(key, result.Identity.PublicKey);
		EXPECT_EQ("11.22.33.44", result.Identity.Host);
		EXPECT_EQ(pSocket, result.pPeerSocket);
	}

	TEST(TEST_CLASS, Ex_CanCreateDefaultResultFromCodeAndKeyAndSocket_Error) {
		// Act:
		auto key = test::GenerateRandomByteArray<Key>();
		auto pSocket = std::make_shared<mocks::MockPacketSocket>();
		auto result = PeerConnectResultEx(PeerConnectCode::Already_Connected, { key, "11.22.33.44" }, pSocket);

		// Assert:
		EXPECT_EQ(PeerConnectCode::Already_Connected, result.Code);
		EXPECT_EQ(Key(), result.Identity.PublicKey);
		EXPECT_EQ("", result.Identity.Host);
		EXPECT_FALSE(!!result.pPeerSocket);
	}

	// endregion
}}
