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

#include "catapult/ionet/NodeInfo.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS NodeInfoTests

	namespace {
		NodeInfo::ServiceIdentifiers ToServiceIdentifiers(std::initializer_list<ServiceIdentifier::ValueType> values) {
			NodeInfo::ServiceIdentifiers ids;
			for (auto value : values)
				ids.insert(ServiceIdentifier(value));

			return ids;
		}
	}

	// region basic

	TEST(TEST_CLASS, CanCreateZeroedConnectionState) {
		// Act:
		ConnectionState connectionState;

		// Assert:
		test::AssertZeroed(connectionState);
	}

	TEST(TEST_CLASS, CanCreateNodeInfo) {
		// Act:
		NodeInfo nodeInfo(NodeSource::Static);

		// Assert:
		EXPECT_EQ(NodeSource::Static, nodeInfo.source());
		EXPECT_EQ(0u, nodeInfo.numConnectionStates());
		EXPECT_TRUE(nodeInfo.services().empty());
	}

	TEST(TEST_CLASS, CanSetSource) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);

		// Act:
		nodeInfo.source(NodeSource::Dynamic);

		// Assert:
		EXPECT_EQ(NodeSource::Dynamic, nodeInfo.source());
		EXPECT_EQ(0u, nodeInfo.numConnectionStates());
		EXPECT_TRUE(nodeInfo.services().empty());
	}

	// endregion

	// region (provision|get)ConnectionState

	TEST(TEST_CLASS, CanAddConnectionState) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);

		// Act:
		auto& connectionState = nodeInfo.provisionConnectionState(ServiceIdentifier(123));

		// Assert:
		EXPECT_EQ(1u, nodeInfo.numConnectionStates());
		EXPECT_EQ(ToServiceIdentifiers({ 123 }), nodeInfo.services());
		test::AssertZeroed(connectionState);
	}

	TEST(TEST_CLASS, CanAddMultipleConnectionStates) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);

		// Act:
		nodeInfo.provisionConnectionState(ServiceIdentifier(123)).Age = 1;
		nodeInfo.provisionConnectionState(ServiceIdentifier(122)).Age = 2;
		nodeInfo.provisionConnectionState(ServiceIdentifier(124)).Age = 3;

		// Assert: three unique connection states were added
		EXPECT_EQ(3u, nodeInfo.numConnectionStates());
		EXPECT_EQ(ToServiceIdentifiers({ 122, 123, 124 }), nodeInfo.services());
		EXPECT_EQ(1u, nodeInfo.getConnectionState(ServiceIdentifier(123))->Age);
		EXPECT_EQ(2u, nodeInfo.getConnectionState(ServiceIdentifier(122))->Age);
		EXPECT_EQ(3u, nodeInfo.getConnectionState(ServiceIdentifier(124))->Age);
	}

	TEST(TEST_CLASS, ProvisionReturnsExistingStateWhenPresent) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);
		nodeInfo.provisionConnectionState(ServiceIdentifier(122));
		const auto& originalConnectionState = nodeInfo.provisionConnectionState(ServiceIdentifier(124));

		// Act:
		const auto& connectionState = nodeInfo.provisionConnectionState(ServiceIdentifier(124));

		// Assert:
		EXPECT_EQ(&originalConnectionState, &connectionState);
	}

	TEST(TEST_CLASS, GetReturnsNullptrWhenMatchingStateIsNotFound) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);
		nodeInfo.provisionConnectionState(ServiceIdentifier(122));
		nodeInfo.provisionConnectionState(ServiceIdentifier(124));

		// Act:
		const auto* pConnectionState = nodeInfo.getConnectionState(ServiceIdentifier(123));

		// Assert:
		EXPECT_FALSE(!!pConnectionState);
	}

	TEST(TEST_CLASS, GetReturnsExistingStateWhenPresent) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);
		nodeInfo.provisionConnectionState(ServiceIdentifier(122));
		const auto& originalConnectionState = nodeInfo.provisionConnectionState(ServiceIdentifier(124));

		// Act:
		const auto* pConnectionState = nodeInfo.getConnectionState(ServiceIdentifier(124));

		// Assert:
		EXPECT_EQ(&originalConnectionState, pConnectionState);
	}

	// endregion

	// region hasActiveConnection

	TEST(TEST_CLASS, HasActiveConnectionReturnsFalseIfThereAreNoConnections) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);

		// Act + Assert:
		EXPECT_FALSE(nodeInfo.hasActiveConnection());
	}

	TEST(TEST_CLASS, HasActiveConnectionReturnsFalseIfAllConnectionsHaveZeroAge) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);
		nodeInfo.provisionConnectionState(ServiceIdentifier(123)).Age = 0;
		nodeInfo.provisionConnectionState(ServiceIdentifier(322)).Age = 0;
		nodeInfo.provisionConnectionState(ServiceIdentifier(224)).Age = 0;

		// Act + Assert:
		EXPECT_FALSE(nodeInfo.hasActiveConnection());
	}

	TEST(TEST_CLASS, HasActiveConnectionReturnsTrueIfSomeConnectionsHaveNonzeroAge) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);
		nodeInfo.provisionConnectionState(ServiceIdentifier(123)).Age = 0;
		nodeInfo.provisionConnectionState(ServiceIdentifier(322)).Age = 987;
		nodeInfo.provisionConnectionState(ServiceIdentifier(224)).Age = 0;

		// Act + Assert:
		EXPECT_TRUE(nodeInfo.hasActiveConnection());
	}

	TEST(TEST_CLASS, HasActiveConnectionReturnsTrueIfAllConnectionsHaveNonzeroAge) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);
		nodeInfo.provisionConnectionState(ServiceIdentifier(123)).Age = 12;
		nodeInfo.provisionConnectionState(ServiceIdentifier(322)).Age = 87;
		nodeInfo.provisionConnectionState(ServiceIdentifier(224)).Age = 56;

		// Act + Assert:
		EXPECT_TRUE(nodeInfo.hasActiveConnection());
	}

	// endregion

	// region clearAge

	TEST(TEST_CLASS, ClearAgeClearsAgeOfKnownServiceIdentifier) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);
		nodeInfo.provisionConnectionState(ServiceIdentifier(123)).Age = 12;
		nodeInfo.provisionConnectionState(ServiceIdentifier(322)).Age = 87;
		nodeInfo.provisionConnectionState(ServiceIdentifier(224)).Age = 56;

		// Act:
		nodeInfo.clearAge(ServiceIdentifier(322));

		// Assert:
		EXPECT_EQ(3u, nodeInfo.numConnectionStates());
		EXPECT_EQ(ToServiceIdentifiers({ 123, 224, 322 }), nodeInfo.services());
		EXPECT_EQ(12u, nodeInfo.getConnectionState(ServiceIdentifier(123))->Age);
		EXPECT_EQ(0u, nodeInfo.getConnectionState(ServiceIdentifier(322))->Age);
		EXPECT_EQ(56u, nodeInfo.getConnectionState(ServiceIdentifier(224))->Age);
	}

	TEST(TEST_CLASS, ClearAgeIgnoresUnknownServiceIdentifier) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);
		nodeInfo.provisionConnectionState(ServiceIdentifier(123)).Age = 12;
		nodeInfo.provisionConnectionState(ServiceIdentifier(322)).Age = 87;
		nodeInfo.provisionConnectionState(ServiceIdentifier(224)).Age = 56;

		// Act:
		nodeInfo.clearAge(ServiceIdentifier(222));

		// Assert:
		EXPECT_EQ(3u, nodeInfo.numConnectionStates());
		EXPECT_EQ(ToServiceIdentifiers({ 123, 224, 322 }), nodeInfo.services());
		EXPECT_EQ(12u, nodeInfo.getConnectionState(ServiceIdentifier(123))->Age);
		EXPECT_EQ(87u, nodeInfo.getConnectionState(ServiceIdentifier(322))->Age);
		EXPECT_EQ(56u, nodeInfo.getConnectionState(ServiceIdentifier(224))->Age);
	}

	// endregion
}}
