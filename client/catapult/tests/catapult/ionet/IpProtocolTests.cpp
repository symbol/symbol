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

#include "catapult/ionet/IpProtocol.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS IpProtocolTests

	TEST(TEST_CLASS, CanMapNodeRolesToIpProtocols) {
		EXPECT_EQ(IpProtocol::IPv4, MapNodeRolesToIpProtocols(NodeRoles::None));
		EXPECT_EQ(IpProtocol::IPv4, MapNodeRolesToIpProtocols(NodeRoles::IPv4));
		EXPECT_EQ(IpProtocol::IPv6, MapNodeRolesToIpProtocols(NodeRoles::IPv6));
		EXPECT_EQ(IpProtocol::IPv4 | IpProtocol::IPv6, MapNodeRolesToIpProtocols(NodeRoles::IPv4 | NodeRoles::IPv6));
	}

	TEST(TEST_CLASS, HasAnyProtocolReturnsCorrectValue_None) {
		EXPECT_FALSE(HasAnyProtocol(IpProtocol::None, NodeRoles::None));
		EXPECT_FALSE(HasAnyProtocol(IpProtocol::None, NodeRoles::IPv4));
		EXPECT_FALSE(HasAnyProtocol(IpProtocol::None, NodeRoles::IPv6));
		EXPECT_FALSE(HasAnyProtocol(IpProtocol::None, NodeRoles::IPv4 | NodeRoles::IPv6));
	}

	TEST(TEST_CLASS, HasAnyProtocolReturnsCorrectValue_IPv4) {
		EXPECT_TRUE(HasAnyProtocol(IpProtocol::IPv4, NodeRoles::None));
		EXPECT_TRUE(HasAnyProtocol(IpProtocol::IPv4, NodeRoles::IPv4));
		EXPECT_FALSE(HasAnyProtocol(IpProtocol::IPv4, NodeRoles::IPv6));
		EXPECT_TRUE(HasAnyProtocol(IpProtocol::IPv4, NodeRoles::IPv4 | NodeRoles::IPv6));
	}

	TEST(TEST_CLASS, HasAnyProtocolReturnsCorrectValue_IPv6) {
		EXPECT_FALSE(HasAnyProtocol(IpProtocol::IPv6, NodeRoles::None));
		EXPECT_FALSE(HasAnyProtocol(IpProtocol::IPv6, NodeRoles::IPv4));
		EXPECT_TRUE(HasAnyProtocol(IpProtocol::IPv6, NodeRoles::IPv6));
		EXPECT_TRUE(HasAnyProtocol(IpProtocol::IPv6, NodeRoles::IPv4 | NodeRoles::IPv6));
	}

	TEST(TEST_CLASS, HasAnyProtocolReturnsCorrectValue_All) {
		EXPECT_TRUE(HasAnyProtocol(IpProtocol::All, NodeRoles::None));
		EXPECT_TRUE(HasAnyProtocol(IpProtocol::All, NodeRoles::IPv4));
		EXPECT_TRUE(HasAnyProtocol(IpProtocol::All, NodeRoles::IPv6));
		EXPECT_TRUE(HasAnyProtocol(IpProtocol::All, NodeRoles::IPv4 | NodeRoles::IPv6));
	}
}}
