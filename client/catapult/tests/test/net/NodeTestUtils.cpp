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
	}
}}
