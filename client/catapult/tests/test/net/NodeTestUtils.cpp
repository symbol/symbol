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

#include "NodeTestUtils.h"
#include "catapult/utils/Casting.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	std::ostream& operator<<(std::ostream& out, const BasicNodeData& data) {
		out << data.Name << " (source " << data.Source << ") " << utils::HexFormat(data.IdentityKey);
		return out;
	}

	BasicNodeDataContainer CollectAll(const ionet::NodeContainerView& view) {
		BasicNodeDataContainer basicDataContainer;
		view.forEach([&basicDataContainer](const auto& node, const auto& nodeInfo) {
			basicDataContainer.insert({ node.identityKey(), node.metadata().Name, nodeInfo.source() });
		});

		return basicDataContainer;
	}

	void AssertZeroed(const ionet::ConnectionState& connectionState) {
		// Assert:
		EXPECT_EQ(0u, connectionState.Age);
		EXPECT_EQ(0u, connectionState.NumAttempts);
		EXPECT_EQ(0u, connectionState.NumSuccesses);
		EXPECT_EQ(0u, connectionState.NumFailures);

		EXPECT_EQ(0u, connectionState.NumConsecutiveFailures);
		EXPECT_EQ(0u, connectionState.BanAge);
	}
}}
