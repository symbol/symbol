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

		auto interactions = nodeInfo.interactions(Timestamp());
		EXPECT_EQ(0u, interactions.NumSuccesses);
		EXPECT_EQ(0u, interactions.NumFailures);

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

	// region addNodeInteraction

	namespace {
		template<typename TIncrement>
		void AssertCanAddInteraction(uint32_t successesPerIncrement, uint32_t failuresPerIncrement, TIncrement increment) {
			// Arrange:
			NodeInfo nodeInfo(NodeSource::Static);
			auto time1 = Timestamp();
			auto time2 = Timestamp(NodeInteractionsContainer::BucketDuration().millis());
			auto time3 = Timestamp(NodeInteractionsContainer::InteractionDuration().millis());

			// Act:
			increment(nodeInfo, time1); // creates bucket 1, active (time1)
			increment(nodeInfo, time2); // creates bucket 2, active (time1, time2)
			increment(nodeInfo, time2); // (time1, time2, time2)
			increment(nodeInfo, time3); // creates bucket 3, prunes bucket 1, active (time2, time2, time3)
			increment(nodeInfo, time3); // (time2, time2, time3, time3)
			increment(nodeInfo, time3); // (time2, time2, time3, time3, time3)

			// Assert: if pruning did not occur, these would not all be the same
			test::AssertNodeInteractions(5 * successesPerIncrement, 5 * failuresPerIncrement, nodeInfo.interactions(time1), "time 1");
			test::AssertNodeInteractions(5 * successesPerIncrement, 5 * failuresPerIncrement, nodeInfo.interactions(time2), "time 2");
			test::AssertNodeInteractions(5 * successesPerIncrement, 5 * failuresPerIncrement, nodeInfo.interactions(time3), "time 3");
		}
	}

	TEST(TEST_CLASS, CanAddSuccessfulNodeInteractionWithAutoPruning) {
		AssertCanAddInteraction(1, 0, [](auto& nodeInfo, auto timestamp) { nodeInfo.incrementSuccesses(timestamp); });
	}

	TEST(TEST_CLASS, CanAddFailureNodeInteractionWithAutoPruning) {
		AssertCanAddInteraction(0, 1, [](auto& nodeInfo, auto timestamp) { nodeInfo.incrementFailures(timestamp); });
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

	TEST(TEST_CLASS, HasActiveConnectionReturnsFalseWhenThereAreNoConnections) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);

		// Act + Assert:
		EXPECT_FALSE(nodeInfo.hasActiveConnection());
	}

	TEST(TEST_CLASS, HasActiveConnectionReturnsFalseWhenAllConnectionsHaveZeroAge) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);
		nodeInfo.provisionConnectionState(ServiceIdentifier(123)).Age = 0;
		nodeInfo.provisionConnectionState(ServiceIdentifier(322)).Age = 0;
		nodeInfo.provisionConnectionState(ServiceIdentifier(224)).Age = 0;

		// Act + Assert:
		EXPECT_FALSE(nodeInfo.hasActiveConnection());
	}

	TEST(TEST_CLASS, HasActiveConnectionReturnsTrueWhenSomeConnectionsHaveNonzeroAge) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);
		nodeInfo.provisionConnectionState(ServiceIdentifier(123)).Age = 0;
		nodeInfo.provisionConnectionState(ServiceIdentifier(322)).Age = 987;
		nodeInfo.provisionConnectionState(ServiceIdentifier(224)).Age = 0;

		// Act + Assert:
		EXPECT_TRUE(nodeInfo.hasActiveConnection());
	}

	TEST(TEST_CLASS, HasActiveConnectionReturnsTrueWhenAllConnectionsHaveNonzeroAge) {
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

	TEST(TEST_CLASS, ClearAgeOnlyUpdatesKnownServiceIdentifier) {
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

	// region updateBan

	namespace {
		void SetBanAge(ConnectionState& connectionState, uint32_t banAge, uint32_t numConsecutiveFailures = 4) {
			connectionState.BanAge = banAge;
			connectionState.NumConsecutiveFailures = numConsecutiveFailures;
		}
	}

	TEST(TEST_CLASS, UpdateBanOnlyUpdatesKnownServiceIdentifier) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);
		SetBanAge(nodeInfo.provisionConnectionState(ServiceIdentifier(123)), 12);
		SetBanAge(nodeInfo.provisionConnectionState(ServiceIdentifier(322)), 87);
		SetBanAge(nodeInfo.provisionConnectionState(ServiceIdentifier(224)), 56);

		// Act:
		nodeInfo.updateBan(ServiceIdentifier(322), 100, 4);

		// Assert:
		EXPECT_EQ(3u, nodeInfo.numConnectionStates());
		EXPECT_EQ(ToServiceIdentifiers({ 123, 224, 322 }), nodeInfo.services());
		EXPECT_EQ(12u, nodeInfo.getConnectionState(ServiceIdentifier(123))->BanAge);
		EXPECT_EQ(88u, nodeInfo.getConnectionState(ServiceIdentifier(322))->BanAge);
		EXPECT_EQ(56u, nodeInfo.getConnectionState(ServiceIdentifier(224))->BanAge);
	}

	TEST(TEST_CLASS, UpdateBanIgnoresUnknownServiceIdentifier) {
		// Arrange:
		NodeInfo nodeInfo(NodeSource::Static);
		SetBanAge(nodeInfo.provisionConnectionState(ServiceIdentifier(123)), 12);
		SetBanAge(nodeInfo.provisionConnectionState(ServiceIdentifier(322)), 87);
		SetBanAge(nodeInfo.provisionConnectionState(ServiceIdentifier(224)), 56);

		// Act:
		nodeInfo.updateBan(ServiceIdentifier(222), 100, 4);

		// Assert:
		EXPECT_EQ(3u, nodeInfo.numConnectionStates());
		EXPECT_EQ(ToServiceIdentifiers({ 123, 224, 322 }), nodeInfo.services());
		EXPECT_EQ(12u, nodeInfo.getConnectionState(ServiceIdentifier(123))->BanAge);
		EXPECT_EQ(87u, nodeInfo.getConnectionState(ServiceIdentifier(322))->BanAge);
		EXPECT_EQ(56u, nodeInfo.getConnectionState(ServiceIdentifier(224))->BanAge);
	}

	namespace {
		void AssertUpdateBan(
				uint32_t banAge,
				uint32_t numConsecutiveFailures,
				uint32_t expectedBanAge,
				uint32_t expectedNumConsecutiveFailures) {
			// Arrange:
			NodeInfo nodeInfo(NodeSource::Static);
			SetBanAge(nodeInfo.provisionConnectionState(ServiceIdentifier(322)), banAge, numConsecutiveFailures);

			// Act:
			nodeInfo.updateBan(ServiceIdentifier(322), 100, 4);

			// Assert:
			EXPECT_EQ(1u, nodeInfo.numConnectionStates());
			EXPECT_EQ(ToServiceIdentifiers({ 322 }), nodeInfo.services());
			EXPECT_EQ(expectedBanAge, nodeInfo.getConnectionState(ServiceIdentifier(322))->BanAge);
			EXPECT_EQ(expectedNumConsecutiveFailures, nodeInfo.getConnectionState(ServiceIdentifier(322))->NumConsecutiveFailures);
		}
	}

	TEST(TEST_CLASS, UpdateBanIncrementsBanAgeOfKnownServiceIdentifierWithExactConsecutiveFailures) {
		AssertUpdateBan(87, 4, 88, 4);
	}

	TEST(TEST_CLASS, UpdateBanIncrementsBanAgeOfKnownServiceIdentifierWithTooManyConsecutiveFailures) {
		AssertUpdateBan(87, 5, 88, 5);
	}

	TEST(TEST_CLASS, UpdateBanClearsBanOfKnownServiceIdentifierWithTooFewConsecutiveFailures) {
		AssertUpdateBan(87, 3, 0, 3);
	}

	TEST(TEST_CLASS, UpdateBanClearsBanOfKnownServiceIdentifierWithMaximumBanAge) {
		AssertUpdateBan(100, 5, 0, 0);
	}

	// endregion
}}
