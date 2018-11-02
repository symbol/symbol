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

#include "catapult/handlers/DiagnosticHandlers.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/ionet/PackedNodeInfo.h"
#include "catapult/model/DiagnosticCounterValue.h"
#include "catapult/utils/DiagnosticCounter.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS DiagnosticHandlersTests

	namespace {
		using CountersVector = std::vector<utils::DiagnosticCounter>;

		void AssertNoResponseWhenPacketIsMalformed(const ionet::ServerPacketHandlers& handlers, ionet::PacketType packetType) {
			// Arrange: malform the packet
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
			pPacket->Type = packetType;
			++pPacket->Size;

			// Act:
			ionet::ServerPacketHandlerContext context({}, "");
			EXPECT_TRUE(handlers.process(*pPacket, context));

			// Assert: malformed packet is ignored
			test::AssertNoResponse(context);
		}
	}

	// region DiagnosticCountersHandler

	TEST(TEST_CLASS, DiagnosticCountersHandler_DoesNotRespondToMalformedRequest) {
		// Arrange:
		ionet::ServerPacketHandlers handlers;
		CountersVector counters;
		RegisterDiagnosticCountersHandler(handlers, counters);

		// Act + Assert:
		AssertNoResponseWhenPacketIsMalformed(handlers, ionet::PacketType::Diagnostic_Counters);
	}

	namespace {
		template<typename TAssertHandlerContext>
		void AssertDiagnosticCountersHandlerWritesCountsInResponseToValidRequest(
				const CountersVector& counters,
				TAssertHandlerContext assertHandlerContext) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			RegisterDiagnosticCountersHandler(handlers, counters);

			// - create a valid request
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
			pPacket->Type = ionet::PacketType::Diagnostic_Counters;

			// Act:
			ionet::ServerPacketHandlerContext context({}, "");
			EXPECT_TRUE(handlers.process(*pPacket, context));

			// Assert: header is correct
			auto expectedPacketSize = sizeof(ionet::PacketHeader) + counters.size() * sizeof(model::DiagnosticCounterValue);
			test::AssertPacketHeader(context, expectedPacketSize, ionet::PacketType::Diagnostic_Counters);

			// - counters are written
			assertHandlerContext(context);
		}
	}

	TEST(TEST_CLASS, DiagnosticCountersHandler_WritesCountsInResponseToValidRequest_ZeroCounters) {
		// Arrange:
		auto counters = CountersVector();

		// Assert:
		AssertDiagnosticCountersHandlerWritesCountsInResponseToValidRequest(counters, [](const auto& context) {
			EXPECT_TRUE(context.response().buffers().empty());
		});
	}

	TEST(TEST_CLASS, DiagnosticCountersHandler_WritesCountsInResponseToValidRequest_SingleCounter) {
		// Arrange:
		auto counters = CountersVector{ utils::DiagnosticCounter(utils::DiagnosticCounterId(123), []() { return 7; }) };

		// Assert:
		AssertDiagnosticCountersHandlerWritesCountsInResponseToValidRequest(counters, [](const auto& context) {
			const auto* pCounterValue = reinterpret_cast<const model::DiagnosticCounterValue*>(test::GetSingleBufferData(context));
			EXPECT_EQ(123u, pCounterValue->Id);
			EXPECT_EQ(7u, pCounterValue->Value);
		});
	}

	TEST(TEST_CLASS, DiagnosticCountersHandler_WritesCountsInResponseToValidRequest_MultipleCounters) {
		// Arrange:
		auto counters = CountersVector{
			utils::DiagnosticCounter(utils::DiagnosticCounterId(123), []() { return 7; }),
			utils::DiagnosticCounter(utils::DiagnosticCounterId(777), []() { return 88; }),
			utils::DiagnosticCounter(utils::DiagnosticCounterId(225), []() { return 222; }),
		};

		// Asssert:
		AssertDiagnosticCountersHandlerWritesCountsInResponseToValidRequest(counters, [](const auto& context) {
			const auto* pCounterValue = reinterpret_cast<const model::DiagnosticCounterValue*>(test::GetSingleBufferData(context));
			EXPECT_EQ(123u, pCounterValue->Id);
			EXPECT_EQ(7u, pCounterValue->Value);

			++pCounterValue;
			EXPECT_EQ(777u, pCounterValue->Id);
			EXPECT_EQ(88u, pCounterValue->Value);

			++pCounterValue;
			EXPECT_EQ(225u, pCounterValue->Id);
			EXPECT_EQ(222u, pCounterValue->Value);
		});
	}

	// endregion

	// region DiagnosticNodesHandler

	TEST(TEST_CLASS, DiagnosticNodesHandler_DoesNotRespondToMalformedRequest) {
		// Arrange:
		ionet::ServerPacketHandlers handlers;
		ionet::NodeContainer nodeContainer;
		RegisterDiagnosticNodesHandler(handlers, nodeContainer);

		// Act + Assert:
		AssertNoResponseWhenPacketIsMalformed(handlers, ionet::PacketType::Active_Node_Infos);
	}

	namespace {
		template<typename TAssertHandlerContext>
		void AssertDiagnosticNodesHandlerWritesCountsInResponseToValidRequest(
				const ionet::NodeContainer& nodeContainer,
				size_t expectedPayloadSize,
				TAssertHandlerContext assertHandlerContext) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			RegisterDiagnosticNodesHandler(handlers, nodeContainer);

			// - create a valid request
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
			pPacket->Type = ionet::PacketType::Active_Node_Infos;

			// Act:
			ionet::ServerPacketHandlerContext context({}, "");
			EXPECT_TRUE(handlers.process(*pPacket, context));

			// Assert: header is correct
			test::AssertPacketHeader(context, sizeof(ionet::PacketHeader) + expectedPayloadSize, ionet::PacketType::Active_Node_Infos);

			// - counters are written
			assertHandlerContext(context);
		}

		ionet::ConnectionState CreateConnectionState(
				uint32_t age,
				uint32_t numAttempts,
				uint32_t numSuccesses,
				uint32_t numFailures,
				uint32_t numConsecutiveFailures,
				uint32_t banAge) {
			auto connectionState = ionet::ConnectionState();
			connectionState.Age = age;
			connectionState.NumAttempts = numAttempts;
			connectionState.NumSuccesses = numSuccesses;
			connectionState.NumFailures = numFailures;
			connectionState.NumConsecutiveFailures = numConsecutiveFailures;
			connectionState.BanAge = banAge;
			return connectionState;
		}

		const ionet::PackedConnectionState& GetConnectionState(const ionet::PackedNodeInfo& nodeInfo, ionet::ServiceIdentifier serviceId) {
			const auto* pConnectionState = nodeInfo.ConnectionStatesPtr();
			for (auto i = 0u; i < nodeInfo.ConnectionStatesCount; ++i, ++pConnectionState) {
				if (pConnectionState->ServiceId == serviceId)
					return *pConnectionState;
			}

			CATAPULT_THROW_INVALID_ARGUMENT_1("no connection state found for service", serviceId);
		}

		void AssertConnectionState(
				const ionet::PackedNodeInfo& nodeInfo,
				ionet::ServiceIdentifier serviceId,
				uint32_t age,
				uint32_t numAttempts,
				uint32_t numSuccesses,
				uint32_t numFailures,
				uint32_t numConsecutiveFailures,
				uint32_t banAge) {
			// Arrange:
			const auto& connectionState = GetConnectionState(nodeInfo, serviceId);
			auto message = "service id " + std::to_string(serviceId.unwrap());

			// Assert:
			EXPECT_EQ(serviceId, connectionState.ServiceId) << message;
			EXPECT_EQ(age, connectionState.Age) << message;
			EXPECT_EQ(numAttempts, connectionState.NumAttempts) << message;
			EXPECT_EQ(numSuccesses, connectionState.NumSuccesses) << message;
			EXPECT_EQ(numFailures, connectionState.NumFailures) << message;
			EXPECT_EQ(numConsecutiveFailures, connectionState.NumConsecutiveFailures) << message;
			EXPECT_EQ(banAge, connectionState.BanAge) << message;
		}
	}

	TEST(TEST_CLASS, DiagnosticNodesHandler_WritesNodeInformationInResponseToValidRequest_ZeroNodes) {
		// Arrange:
		ionet::NodeContainer nodeContainer;

		// Assert:
		AssertDiagnosticNodesHandlerWritesCountsInResponseToValidRequest(nodeContainer, 0, [](const auto& context) {
			EXPECT_TRUE(context.response().buffers().empty());
		});
	}

	TEST(TEST_CLASS, DiagnosticNodesHandler_WritesNodeInformationInResponseToValidRequest_ZeroActiveNodes) {
		// Arrange: add three inactive nodes
		auto keys = test::GenerateRandomDataVector<Key>(3);
		ionet::NodeContainer nodeContainer;
		{
			auto modifier = nodeContainer.modifier();
			modifier.add(test::CreateNamedNode(keys[0], "a"), ionet::NodeSource::Static);
			modifier.add(test::CreateNamedNode(keys[1], "b"), ionet::NodeSource::Dynamic);
			modifier.add(test::CreateNamedNode(keys[2], "c"), ionet::NodeSource::Local);
		}

		// Assert:
		AssertDiagnosticNodesHandlerWritesCountsInResponseToValidRequest(nodeContainer, 0, [](const auto& context) {
			EXPECT_TRUE(context.response().buffers().empty());
		});
	}

	TEST(TEST_CLASS, DiagnosticNodesHandler_WritesNodeInformationInResponseToValidRequest_SingleActiveNode) {
		// Arrange: add one active and two inactive nodes
		auto keys = test::GenerateRandomDataVector<Key>(3);
		ionet::NodeContainer nodeContainer;
		{
			auto modifier = nodeContainer.modifier();
			modifier.add(test::CreateNamedNode(keys[0], "a"), ionet::NodeSource::Static);
			modifier.add(test::CreateNamedNode(keys[1], "b"), ionet::NodeSource::Dynamic);
			modifier.add(test::CreateNamedNode(keys[2], "c"), ionet::NodeSource::Local);

			// - provision two services (notice that only one is active but both should be serialized)
			modifier.provisionConnectionState(ionet::ServiceIdentifier(123), keys[1]) = CreateConnectionState(0, 2, 3, 4, 5, 6);
			modifier.provisionConnectionState(ionet::ServiceIdentifier(987), keys[1]) = CreateConnectionState(49, 64, 36, 25, 16, 9);
		}

		// Assert:
		auto expectedPacketSize = sizeof(ionet::PackedNodeInfo) + 2 * sizeof(ionet::PackedConnectionState);
		AssertDiagnosticNodesHandlerWritesCountsInResponseToValidRequest(nodeContainer, expectedPacketSize, [&keys](const auto& context) {
			ASSERT_EQ(1u, context.response().buffers().size());

			const auto& nodeInfo = reinterpret_cast<const ionet::PackedNodeInfo&>(*context.response().buffers()[0].pData);
			EXPECT_EQ(keys[1], nodeInfo.IdentityKey);
			EXPECT_EQ(ionet::NodeSource::Dynamic, nodeInfo.Source);
			ASSERT_EQ(2u, nodeInfo.ConnectionStatesCount);
			AssertConnectionState(nodeInfo, ionet::ServiceIdentifier(123), 0, 2, 3, 4, 5, 6);
			AssertConnectionState(nodeInfo, ionet::ServiceIdentifier(987), 49, 64, 36, 25, 16, 9);
		});
	}

	namespace {
		const ionet::PackedNodeInfo& FindByKey(const std::vector<const ionet::PackedNodeInfo*>& nodeInfos, const Key& identityKey) {
			auto iter = std::find_if(nodeInfos.cbegin(), nodeInfos.cend(), [&identityKey](const auto* pNodeInfo) {
				return identityKey == pNodeInfo->IdentityKey;
			});

			if (nodeInfos.cend() == iter)
				CATAPULT_THROW_INVALID_ARGUMENT_1("could not find packet node info with key", utils::HexFormat(identityKey));

			return **iter;
		}
	}

	TEST(TEST_CLASS, DiagnosticNodesHandler_WritesNodeInformationInResponseToValidRequest_MultipleActiveNodes) {
		// Arrange: add three active nodes
		auto keys = test::GenerateRandomDataVector<Key>(3);
		ionet::NodeContainer nodeContainer;
		{
			auto modifier = nodeContainer.modifier();
			modifier.add(test::CreateNamedNode(keys[0], "a"), ionet::NodeSource::Static);
			modifier.add(test::CreateNamedNode(keys[1], "b"), ionet::NodeSource::Dynamic);
			modifier.add(test::CreateNamedNode(keys[2], "c"), ionet::NodeSource::Local);

			// - provision six services
			modifier.provisionConnectionState(ionet::ServiceIdentifier(123), keys[0]) = CreateConnectionState(7, 6, 5, 4, 3, 2);
			modifier.provisionConnectionState(ionet::ServiceIdentifier(888), keys[0]) = CreateConnectionState(0, 5, 4, 3, 2, 1);
			modifier.provisionConnectionState(ionet::ServiceIdentifier(222), keys[0]) = CreateConnectionState(5, 4, 3, 2, 1, 0);

			modifier.provisionConnectionState(ionet::ServiceIdentifier(123), keys[1]) = CreateConnectionState(1, 2, 3, 4, 5, 6);
			modifier.provisionConnectionState(ionet::ServiceIdentifier(987), keys[1]) = CreateConnectionState(49, 64, 36, 25, 16, 9);

			modifier.provisionConnectionState(ionet::ServiceIdentifier(111), keys[2]) = CreateConnectionState(9, 8, 7, 6, 5, 4);
		}

		// Assert:
		auto expectedPacketSize = 3 * sizeof(ionet::PackedNodeInfo) + 6 * sizeof(ionet::PackedConnectionState);
		AssertDiagnosticNodesHandlerWritesCountsInResponseToValidRequest(nodeContainer, expectedPacketSize, [&keys](const auto& context) {
			// - there are three buffers
			const auto& buffers = context.response().buffers();
			ASSERT_EQ(3u, buffers.size());

			std::vector<const ionet::PackedNodeInfo*> nodeInfos;
			for (const auto& buffer : buffers)
				nodeInfos.push_back(reinterpret_cast<const ionet::PackedNodeInfo*>(buffer.pData));

			// - the buffers have correct data in any order
			const auto& nodeInfo1 = FindByKey(nodeInfos, keys[0]);
			EXPECT_EQ(keys[0], nodeInfo1.IdentityKey);
			EXPECT_EQ(ionet::NodeSource::Static, nodeInfo1.Source);
			ASSERT_EQ(3u, nodeInfo1.ConnectionStatesCount);
			AssertConnectionState(nodeInfo1, ionet::ServiceIdentifier(123), 7, 6, 5, 4, 3, 2);
			AssertConnectionState(nodeInfo1, ionet::ServiceIdentifier(888), 0, 5, 4, 3, 2, 1);
			AssertConnectionState(nodeInfo1, ionet::ServiceIdentifier(222), 5, 4, 3, 2, 1, 0);

			const auto& nodeInfo2 = FindByKey(nodeInfos, keys[1]);
			EXPECT_EQ(keys[1], nodeInfo2.IdentityKey);
			EXPECT_EQ(ionet::NodeSource::Dynamic, nodeInfo2.Source);
			ASSERT_EQ(2u, nodeInfo2.ConnectionStatesCount);
			AssertConnectionState(nodeInfo2, ionet::ServiceIdentifier(123), 1, 2, 3, 4, 5, 6);
			AssertConnectionState(nodeInfo2, ionet::ServiceIdentifier(987), 49, 64, 36, 25, 16, 9);

			const auto& nodeInfo3 = FindByKey(nodeInfos, keys[2]);
			EXPECT_EQ(keys[2], nodeInfo3.IdentityKey);
			EXPECT_EQ(ionet::NodeSource::Local, nodeInfo3.Source);
			ASSERT_EQ(1u, nodeInfo3.ConnectionStatesCount);
			AssertConnectionState(nodeInfo3, ionet::ServiceIdentifier(111), 9, 8, 7, 6, 5, 4);
		});
	}

	// endregion
}}
