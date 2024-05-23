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

#include "catapult/ionet/NodeRoles.h"
#include "tests/TestHarness.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"

namespace catapult {
namespace ionet {

#define TEST_CLASS NodeRolesTests

    TEST(TEST_CLASS, CanParseValidNodeRoles)
    {
        test::AssertParse("Peer", NodeRoles::Peer, TryParseValue);
        test::AssertParse("Api", NodeRoles::Api, TryParseValue);
        test::AssertParse("Voting", NodeRoles::Voting, TryParseValue);

        test::AssertParse("IPv4", NodeRoles::IPv4, TryParseValue);
        test::AssertParse("IPv6", NodeRoles::IPv6, TryParseValue);

        test::AssertParse("Peer,Api", NodeRoles::Peer | NodeRoles::Api, TryParseValue);
        test::AssertParse("IPv6,Api", NodeRoles::IPv6 | NodeRoles::Api, TryParseValue);
        test::AssertParse("IPv4,IPv6,Api", NodeRoles::IPv4 | NodeRoles::IPv6 | NodeRoles::Api, TryParseValue);
    }
}
}
