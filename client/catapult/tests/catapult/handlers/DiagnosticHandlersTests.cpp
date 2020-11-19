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

#include "catapult/handlers/DiagnosticHandlers.h"
#include "catapult/api/ChainPackets.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/ionet/NodeInteractionResult.h"
#include "catapult/ionet/PackedNodeInfo.h"
#include "catapult/model/DiagnosticCounterValue.h"
#include "catapult/utils/DiagnosticCounter.h"
#include "tests/catapult/handlers/test/HeightRequestHandlerTests.h"
#include "tests/test/core/BlockStatementTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/test/net/NodeTestUtils.h"

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
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert: malformed packet is ignored
			test::AssertNoResponse(handlerContext);
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
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert: header is correct
			auto expectedPacketSize = sizeof(ionet::PacketHeader) + counters.size() * sizeof(model::DiagnosticCounterValue);
			test::AssertPacketHeader(handlerContext, expectedPacketSize, ionet::PacketType::Diagnostic_Counters);

			// - counters are written
			assertHandlerContext(handlerContext);
		}
	}

	TEST(TEST_CLASS, DiagnosticCountersHandler_WritesCountsInResponseToValidRequest_ZeroCounters) {
		// Arrange:
		auto counters = CountersVector();

		// Assert:
		AssertDiagnosticCountersHandlerWritesCountsInResponseToValidRequest(counters, [](const auto& handlerContext) {
			EXPECT_TRUE(handlerContext.response().buffers().empty());
		});
	}

	TEST(TEST_CLASS, DiagnosticCountersHandler_WritesCountsInResponseToValidRequest_SingleCounter) {
		// Arrange:
		auto counters = CountersVector{ utils::DiagnosticCounter(utils::DiagnosticCounterId(123), []() { return 7; }) };

		// Assert:
		AssertDiagnosticCountersHandlerWritesCountsInResponseToValidRequest(counters, [](const auto& handlerContext) {
			const auto* pCounterValue = reinterpret_cast<const model::DiagnosticCounterValue*>(test::GetSingleBufferData(handlerContext));
			EXPECT_EQ(123u, pCounterValue->Id);
			EXPECT_EQ(7u, pCounterValue->Value);
		});
	}

	TEST(TEST_CLASS, DiagnosticCountersHandler_WritesCountsInResponseToValidRequest_MultipleCounters) {
		// Arrange:
		auto counters = CountersVector{
			utils::DiagnosticCounter(utils::DiagnosticCounterId(123), []() { return 7; }),
			utils::DiagnosticCounter(utils::DiagnosticCounterId(777), []() { return 88; }),
			utils::DiagnosticCounter(utils::DiagnosticCounterId(225), []() { return 222; })
		};

		// Asssert:
		AssertDiagnosticCountersHandlerWritesCountsInResponseToValidRequest(counters, [](const auto& handlerContext) {
			const auto* pCounterValue = reinterpret_cast<const model::DiagnosticCounterValue*>(test::GetSingleBufferData(handlerContext));
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
		model::NodeIdentity ToIdentity(const Key& identityKey) {
			return { identityKey, "11.22.33.44" };
		}

		ionet::Node CreateNamedNode(const Key& identityKey, const std::string& name) {
			return test::CreateNamedNode({ identityKey, "11.22.33.44" }, name);
		}

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
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert: header is correct
			test::AssertPacketHeader(
					handlerContext,
					sizeof(ionet::PacketHeader) + expectedPayloadSize,
					ionet::PacketType::Active_Node_Infos);

			// - counters are written
			assertHandlerContext(handlerContext);
		}

		ionet::ConnectionState CreateConnectionState(uint32_t age, uint32_t numConsecutiveFailures, uint32_t banAge) {
			auto connectionState = ionet::ConnectionState();
			connectionState.Age = age;
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
				uint32_t numConsecutiveFailures,
				uint32_t banAge) {
			// Arrange:
			const auto& connectionState = GetConnectionState(nodeInfo, serviceId);
			auto message = "service id " + std::to_string(serviceId.unwrap());

			// Assert:
			EXPECT_EQ(serviceId, connectionState.ServiceId) << message;
			EXPECT_EQ(age, connectionState.Age) << message;
			EXPECT_EQ(numConsecutiveFailures, connectionState.NumConsecutiveFailures) << message;
			EXPECT_EQ(banAge, connectionState.BanAge) << message;
		}
	}

	TEST(TEST_CLASS, DiagnosticNodesHandler_WritesNodeInformationInResponseToValidRequest_ZeroNodes) {
		// Arrange:
		ionet::NodeContainer nodeContainer;

		// Assert:
		AssertDiagnosticNodesHandlerWritesCountsInResponseToValidRequest(nodeContainer, 0, [](const auto& handlerContext) {
			EXPECT_TRUE(handlerContext.response().buffers().empty());
		});
	}

	TEST(TEST_CLASS, DiagnosticNodesHandler_WritesNodeInformationInResponseToValidRequest_ZeroActiveNodes) {
		// Arrange: add three inactive nodes
		auto keys = test::GenerateRandomDataVector<Key>(3);
		ionet::NodeContainer nodeContainer;
		{
			auto modifier = nodeContainer.modifier();
			modifier.add(CreateNamedNode(keys[0], "a"), ionet::NodeSource::Static);
			modifier.add(CreateNamedNode(keys[1], "b"), ionet::NodeSource::Dynamic);
			modifier.add(CreateNamedNode(keys[2], "c"), ionet::NodeSource::Local);
		}

		// Assert:
		AssertDiagnosticNodesHandlerWritesCountsInResponseToValidRequest(nodeContainer, 0, [](const auto& handlerContext) {
			EXPECT_TRUE(handlerContext.response().buffers().empty());
		});
	}

	TEST(TEST_CLASS, DiagnosticNodesHandler_WritesNodeInformationInResponseToValidRequest_SingleActiveNode) {
		// Arrange: add one active and two inactive nodes
		auto keys = test::GenerateRandomDataVector<Key>(3);
		ionet::NodeContainer nodeContainer;
		{
			auto modifier = nodeContainer.modifier();
			modifier.add(CreateNamedNode(keys[0], "a"), ionet::NodeSource::Static);
			modifier.add(CreateNamedNode(keys[1], "b"), ionet::NodeSource::Dynamic);
			modifier.add(CreateNamedNode(keys[2], "c"), ionet::NodeSource::Local);

			// - add some node interaction results
			modifier.incrementSuccesses(ToIdentity(keys[1]));
			modifier.incrementSuccesses(ToIdentity(keys[1]));
			modifier.incrementFailures(ToIdentity(keys[1]));

			// - provision two services (notice that only one is active but both should be serialized)
			modifier.provisionConnectionState(ionet::ServiceIdentifier(123), ToIdentity(keys[1])) = CreateConnectionState(0, 5, 6);
			modifier.provisionConnectionState(ionet::ServiceIdentifier(987), ToIdentity(keys[1])) = CreateConnectionState(49, 16, 9);
		}

		// Assert:
		auto expectedPacketSize = sizeof(ionet::PackedNodeInfo) + 2 * sizeof(ionet::PackedConnectionState);
		AssertDiagnosticNodesHandlerWritesCountsInResponseToValidRequest(nodeContainer, expectedPacketSize, [&keys](
				const auto& handlerContext) {
			ASSERT_EQ(1u, handlerContext.response().buffers().size());

			const auto& nodeInfo = reinterpret_cast<const ionet::PackedNodeInfo&>(*handlerContext.response().buffers()[0].pData);
			EXPECT_EQ(keys[1], nodeInfo.IdentityKey);
			EXPECT_EQ(ionet::NodeSource::Dynamic, nodeInfo.Source);
			EXPECT_EQ(2u, nodeInfo.Interactions.NumSuccesses);
			EXPECT_EQ(1u, nodeInfo.Interactions.NumFailures);
			ASSERT_EQ(2u, nodeInfo.ConnectionStatesCount);
			AssertConnectionState(nodeInfo, ionet::ServiceIdentifier(123), 0, 5, 6);
			AssertConnectionState(nodeInfo, ionet::ServiceIdentifier(987), 49, 16, 9);
		});
	}

	namespace {
		const ionet::PackedNodeInfo& FindByKey(const std::vector<const ionet::PackedNodeInfo*>& nodeInfos, const Key& identityKey) {
			auto iter = std::find_if(nodeInfos.cbegin(), nodeInfos.cend(), [&identityKey](const auto* pNodeInfo) {
				return identityKey == pNodeInfo->IdentityKey;
			});

			if (nodeInfos.cend() == iter)
				CATAPULT_THROW_INVALID_ARGUMENT_1("could not find packet node info with key", identityKey);

			return **iter;
		}
	}

	TEST(TEST_CLASS, DiagnosticNodesHandler_WritesNodeInformationInResponseToValidRequest_MultipleActiveNodes) {
		// Arrange: add three active nodes
		auto keys = test::GenerateRandomDataVector<Key>(3);
		ionet::NodeContainer nodeContainer;
		{
			auto modifier = nodeContainer.modifier();
			modifier.add(CreateNamedNode(keys[0], "a"), ionet::NodeSource::Static);
			modifier.add(CreateNamedNode(keys[1], "b"), ionet::NodeSource::Dynamic);
			modifier.add(CreateNamedNode(keys[2], "c"), ionet::NodeSource::Local);

			// - add some node interaction results
			modifier.incrementSuccesses(ToIdentity(keys[0]));
			modifier.incrementSuccesses(ToIdentity(keys[1]));
			modifier.incrementFailures(ToIdentity(keys[1]));
			modifier.incrementFailures(ToIdentity(keys[2]));

			// - provision six services
			modifier.provisionConnectionState(ionet::ServiceIdentifier(123), ToIdentity(keys[0])) = CreateConnectionState(7, 3, 2);
			modifier.provisionConnectionState(ionet::ServiceIdentifier(888), ToIdentity(keys[0])) = CreateConnectionState(0, 2, 1);
			modifier.provisionConnectionState(ionet::ServiceIdentifier(222), ToIdentity(keys[0])) = CreateConnectionState(5, 1, 0);

			modifier.provisionConnectionState(ionet::ServiceIdentifier(123), ToIdentity(keys[1])) = CreateConnectionState(1, 5, 6);
			modifier.provisionConnectionState(ionet::ServiceIdentifier(987), ToIdentity(keys[1])) = CreateConnectionState(49, 16, 9);

			modifier.provisionConnectionState(ionet::ServiceIdentifier(111), ToIdentity(keys[2])) = CreateConnectionState(9, 5, 4);
		}

		// Assert:
		auto expectedPacketSize = 3 * sizeof(ionet::PackedNodeInfo) + 6 * sizeof(ionet::PackedConnectionState);
		AssertDiagnosticNodesHandlerWritesCountsInResponseToValidRequest(nodeContainer, expectedPacketSize, [&keys](
				const auto& handlerContext) {
			// - there are three buffers
			const auto& buffers = handlerContext.response().buffers();
			ASSERT_EQ(3u, buffers.size());

			std::vector<const ionet::PackedNodeInfo*> nodeInfos;
			for (const auto& buffer : buffers)
				nodeInfos.push_back(reinterpret_cast<const ionet::PackedNodeInfo*>(buffer.pData));

			// - the buffers have correct data in any order
			const auto& nodeInfo1 = FindByKey(nodeInfos, keys[0]);
			EXPECT_EQ(keys[0], nodeInfo1.IdentityKey);
			EXPECT_EQ(ionet::NodeSource::Static, nodeInfo1.Source);
			EXPECT_EQ(1u, nodeInfo1.Interactions.NumSuccesses);
			EXPECT_EQ(0u, nodeInfo1.Interactions.NumFailures);
			ASSERT_EQ(3u, nodeInfo1.ConnectionStatesCount);
			AssertConnectionState(nodeInfo1, ionet::ServiceIdentifier(123), 7, 3, 2);
			AssertConnectionState(nodeInfo1, ionet::ServiceIdentifier(888), 0, 2, 1);
			AssertConnectionState(nodeInfo1, ionet::ServiceIdentifier(222), 5, 1, 0);

			const auto& nodeInfo2 = FindByKey(nodeInfos, keys[1]);
			EXPECT_EQ(keys[1], nodeInfo2.IdentityKey);
			EXPECT_EQ(ionet::NodeSource::Dynamic, nodeInfo2.Source);
			EXPECT_EQ(1u, nodeInfo2.Interactions.NumSuccesses);
			EXPECT_EQ(1u, nodeInfo2.Interactions.NumFailures);
			ASSERT_EQ(2u, nodeInfo2.ConnectionStatesCount);
			AssertConnectionState(nodeInfo2, ionet::ServiceIdentifier(123), 1, 5, 6);
			AssertConnectionState(nodeInfo2, ionet::ServiceIdentifier(987), 49, 16, 9);

			const auto& nodeInfo3 = FindByKey(nodeInfos, keys[2]);
			EXPECT_EQ(keys[2], nodeInfo3.IdentityKey);
			EXPECT_EQ(ionet::NodeSource::Local, nodeInfo3.Source);
			EXPECT_EQ(0u, nodeInfo3.Interactions.NumSuccesses);
			EXPECT_EQ(1u, nodeInfo3.Interactions.NumFailures);
			ASSERT_EQ(1u, nodeInfo3.ConnectionStatesCount);
			AssertConnectionState(nodeInfo3, ionet::ServiceIdentifier(111), 9, 5, 4);
		});
	}

	// endregion

	// region DiagnosticBlockStatementHandler

	namespace {
		using BlockStatementRequestPacket = api::HeightPacket<ionet::PacketType::Block_Statement>;

		struct DiagnosticBlockStatementHandlerTraits {
			static ionet::PacketType ResponsePacketType() {
				return BlockStatementRequestPacket::Packet_Type;
			}

			static auto CreateRequestPacket() {
				return ionet::CreateSharedPacket<BlockStatementRequestPacket>();
			}

			static void Register(ionet::ServerPacketHandlers& handlers, const io::BlockStorageCache& storage) {
				RegisterDiagnosticBlockStatementHandler(handlers, storage);
			}
		};

		template<typename TAssertHandlerContext>
		void AssertBlockStatementHandlerWritesStatementDataInResponseToValidRequest(
				const io::BlockStorageCache& storage,
				size_t blockStatementDataSize,
				TAssertHandlerContext assertHandlerContext) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			RegisterDiagnosticBlockStatementHandler(handlers, storage);

			// - create a valid request
			auto pPacket = ionet::CreateSharedPacket<BlockStatementRequestPacket>();
			pPacket->Height = Height(2);

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert: header is correct
			auto expectedPacketSize = sizeof(ionet::PacketHeader) + blockStatementDataSize;
			test::AssertPacketHeader(handlerContext, expectedPacketSize, ionet::PacketType::Block_Statement);

			const auto& buffers = handlerContext.response().buffers();
			ASSERT_EQ(1u, buffers.size());

			// - block statement is written
			assertHandlerContext(buffers);
		}

		auto BlockToBlockElement(const model::Block& block, const std::vector<size_t>& numStatements) {
			auto blockElement = test::BlockToBlockElement(block, test::GenerateRandomByteArray<Hash256>());
			test::FillWithRandomData(blockElement.GenerationHash);
			blockElement.OptionalStatement = test::GenerateRandomStatements(numStatements);
			return blockElement;
		}

		void AssertWritesBlockStatementDataInResponseToValidRequest(const std::vector<size_t>& numStatements) {
			// Arrange:
			auto pBlock = test::GenerateBlockWithTransactions(5, Height(2));
			auto blockElement = BlockToBlockElement(*pBlock, numStatements);
			io::BlockStorageCache storage(
					std::make_unique<mocks::MockMemoryBlockStorage>(),
					std::make_unique<mocks::MockMemoryBlockStorage>());
			{
				auto modifier = storage.modifier();
				modifier.saveBlock(blockElement);
				modifier.commit();
			}

			// Act + Assert:
			auto expectedData = test::SerializeBlockStatement(*blockElement.OptionalStatement);
			AssertBlockStatementHandlerWritesStatementDataInResponseToValidRequest(storage, expectedData.size(), [&expectedData](
					const auto& buffers) {
				const auto& blockStatementData = buffers[0];
				ASSERT_EQ(expectedData.size(), blockStatementData.Size);
				EXPECT_EQ_MEMORY(expectedData.data(), blockStatementData.pData, expectedData.size());
			});
		}
	}

	DEFINE_HEIGHT_REQUEST_HANDLER_TESTS(TEST_CLASS, DiagnosticBlockStatementHandler)

	TEST(TEST_CLASS, DiagnosticBlockStatementHandler_NoResponseWhenBlockStatementIsNotPresent) {
		// Arrange:
		auto pStorage = mocks::CreateMemoryBlockStorageCache(2);
		ionet::ServerPacketHandlers handlers;
		RegisterDiagnosticBlockStatementHandler(handlers, *pStorage);

		// - create a valid request
		auto pPacket = ionet::CreateSharedPacket<BlockStatementRequestPacket>();
		pPacket->Height = Height(2);

		// Act:
		ionet::ServerPacketHandlerContext handlerContext;
		EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

		// Assert:
		EXPECT_FALSE(handlerContext.hasResponse());
	}

	TEST(TEST_CLASS, DiagnosticBlockStatementHandler_WritesBlockStatementDataInResponseToValidRequest_EmptyBlockStatement) {
		AssertWritesBlockStatementDataInResponseToValidRequest({ 0, 0, 0 });
	}

	TEST(TEST_CLASS, DiagnosticBlockStatementHandler_WritesBlockStatementDataInResponseToValidRequest_BlockStatement) {
		AssertWritesBlockStatementDataInResponseToValidRequest({ 6, 2, 5 });
	}

	// endregion
}}
