#include "nodediscovery/src/NodePingRequestor.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/ConnectionSettings.h"
#include "catapult/net/VerifyPeer.h"
#include "catapult/utils/TimeSpan.h"
#include "nodediscovery/tests/test/NodeDiscoveryTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace nodediscovery {

#define TEST_CLASS NodePingRequestorTests

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;

		net::ConnectionSettings CreateSettingsWithTimeout(const utils::TimeSpan& timeout) {
			auto settings = net::ConnectionSettings();
			settings.Timeout = timeout;
			return settings;
		}

		struct RequestorTestContext {
		public:
			explicit RequestorTestContext(const utils::TimeSpan& timeout = utils::TimeSpan::FromMinutes(1))
					: ClientKeyPair(test::GenerateKeyPair())
					, pPool(test::CreateStartedIoServiceThreadPool())
					, ServerKeyPair(test::GenerateKeyPair())
					, ServerPublicKey(ServerKeyPair.publicKey())
					, pRequestor(CreateNodePingRequestor(pPool, ClientKeyPair, CreateSettingsWithTimeout(timeout), Network_Identifier))
			{}

		public:
			crypto::KeyPair ClientKeyPair;
			std::shared_ptr<thread::IoServiceThreadPool> pPool;

			crypto::KeyPair ServerKeyPair;
			Key ServerPublicKey;

			std::shared_ptr<NodePingRequestor> pRequestor;

		public:
			std::shared_ptr<ionet::Packet> createNodePingResponsePacket(ionet::NodeVersion version, const std::string& name) const {
				auto host = test::CreateLocalHostNodeEndpoint().Host;
				auto pPacket = test::CreateNodePushPingPacket(ServerPublicKey, version, host, name);
				pPacket->Type = ionet::PacketType::Node_Discovery_Pull_Ping;
				return pPacket;
			}

		public:
			void waitForActiveConnections(uint32_t numConnections) const {
				WAIT_FOR_VALUE_EXPR(numConnections, pRequestor->numActiveConnections());
			}
		};

		template<typename TAction>
		void RunConnectedTest(const RequestorTestContext& context, const std::shared_ptr<ionet::Packet>& pResponsePacket, TAction action) {
			// Arrange: create a server for connecting
			test::TcpAcceptor acceptor(context.pPool->service());

			std::shared_ptr<ionet::PacketSocket> pServerSocket;
			test::SpawnPacketServerWork(acceptor, [&context, &pServerSocket, pResponsePacket](const auto& pSocket) {
				pServerSocket = pSocket;
				net::VerifyClient(pSocket, context.ServerKeyPair, [pResponsePacket, pSocket](auto, const auto&) {
					// - write the packet if specified
					if (pResponsePacket)
						pSocket->write(pResponsePacket, [](auto) {});
				});
			});

			// Act: initiate a ping request
			std::atomic<size_t> numCallbacks(0);
			std::vector<std::pair<NodePingResult, ionet::Node>> pingResultPairs;
			auto requestNode = test::CreateLocalHostNode(context.ServerPublicKey);
			context.pRequestor->requestPing(requestNode, [&numCallbacks, &pingResultPairs](auto result, const auto& responseNode) {
				pingResultPairs.emplace_back(result, responseNode);
				++numCallbacks;
			});
			WAIT_FOR_ONE(numCallbacks);

			// Assert:
			ASSERT_EQ(1u, pingResultPairs.size());
			action(*context.pRequestor, pingResultPairs[0].first, pingResultPairs[0].second);

			// - no connections remain
			context.waitForActiveConnections(0);
			EXPECT_EQ(0u, context.pRequestor->numActiveConnections());
		}

		void AssertFailedConnection(const NodePingRequestor& requestor, const ionet::Node& responseNode) {
			// Assert:
			EXPECT_EQ(1u, requestor.numTotalPingRequests());
			EXPECT_EQ(0u, requestor.numSuccessfulPingRequests());
			EXPECT_EQ(Key{}, responseNode.identityKey());
		}
	}

	TEST(TEST_CLASS, RequestPingFailsWhenConnectionIsNotAccepted) {
		// Arrange: create a valid packet
		RequestorTestContext context;
		auto pPacket = context.createNodePingResponsePacket(ionet::NodeVersion(1234), "a");

		// - change the public key to fail verification (verify failures are treated as connection failures)
		test::FillWithRandomData(context.ServerPublicKey);

		// Act:
		RunConnectedTest(context, pPacket, [](const auto& requestor, auto result, const auto& responseNode) {
			// Assert:
			EXPECT_EQ(NodePingResult::Failure_Connection, result);
			AssertFailedConnection(requestor, responseNode);
		});
	}

	TEST(TEST_CLASS, RequestPingFailsWhenConnectionInteractionFails) {
		// Arrange: create an invalid packet (no payload)
		RequestorTestContext context;
		auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
		pPacket->Type = ionet::PacketType::Node_Discovery_Pull_Ping;

		// Act:
		RunConnectedTest(context, pPacket, [](const auto& requestor, auto result, const auto& responseNode) {
			// Assert:
			EXPECT_EQ(NodePingResult::Failure_Interaction, result);
			AssertFailedConnection(requestor, responseNode);
		});
	}

	TEST(TEST_CLASS, RequestPingFailsWhenResponseNodeHasIncompatibleNetwork) {
		// Arrange: create a valid packet with a wrong network
		RequestorTestContext context;
		auto pPacket = context.createNodePingResponsePacket(ionet::NodeVersion(1234), "a");
		reinterpret_cast<ionet::NetworkNode&>(*pPacket->Data()).NetworkIdentifier = model::NetworkIdentifier::Zero;

		// Act:
		RunConnectedTest(context, pPacket, [](const auto& requestor, auto result, const auto& responseNode) {
			// Assert:
			EXPECT_EQ(NodePingResult::Failure_Incompatible, result);
			AssertFailedConnection(requestor, responseNode);
		});
	}

	TEST(TEST_CLASS, RequestPingFailsWhenResponseNodeHasWrongIdentity) {
		// Arrange: create a valid packet with a wrong identity
		RequestorTestContext context;
		auto pPacket = context.createNodePingResponsePacket(ionet::NodeVersion(1234), "a");
		test::FillWithRandomData(reinterpret_cast<ionet::NetworkNode&>(*pPacket->Data()).IdentityKey);

		// Act:
		RunConnectedTest(context, pPacket, [](const auto& requestor, auto result, const auto& responseNode) {
			// Assert:
			EXPECT_EQ(NodePingResult::Failure_Incompatible, result);
			AssertFailedConnection(requestor, responseNode);
		});
	}

	TEST(TEST_CLASS, RequestPingFailsWhenResponseTimesOut) {
		// Arrange:
		RequestorTestContext context(utils::TimeSpan::FromMilliseconds(100));

		// Act: a nullptr packet will prevent any response from being sent
		RunConnectedTest(context, nullptr, [](const auto& requestor, auto result, const auto& responseNode) {
			// Assert:
			EXPECT_EQ(NodePingResult::Failure_Timeout, result);
			AssertFailedConnection(requestor, responseNode);
		});
	}

	TEST(TEST_CLASS, RequestPingSucceedsWhenReturnedNodeIsCompatible) {
		// Arrange:
		RequestorTestContext context;
		auto pPacket = context.createNodePingResponsePacket(ionet::NodeVersion(1234), "a");
		const auto& expectedIdentityKey = context.ServerPublicKey;

		// Act:
		RunConnectedTest(context, pPacket, [&expectedIdentityKey](const auto& requestor, auto result, const auto& responseNode) {
			// Assert:
			EXPECT_EQ(NodePingResult::Success, result);

			EXPECT_EQ(expectedIdentityKey, responseNode.identityKey());
			EXPECT_EQ("127.0.0.1", responseNode.endpoint().Host);
			EXPECT_EQ(ionet::NodeVersion(1234), responseNode.metadata().Version);
			EXPECT_EQ("a", responseNode.metadata().Name);

			EXPECT_EQ(1u, requestor.numTotalPingRequests());
			EXPECT_EQ(1u, requestor.numSuccessfulPingRequests());
		});
	}

	namespace {
		bool RunShutdownTest(const consumer<RequestorTestContext&>& shutdown) {
			// Arrange:
			RequestorTestContext context(utils::TimeSpan::FromMilliseconds(100));

			// - set up a server but don't respond to verify in order to trigger a timeout error
			test::TcpAcceptor acceptor(context.pPool->service());
			std::shared_ptr<ionet::PacketSocket> pServerSocket;
			test::SpawnPacketServerWork(acceptor, [&serverKeyPair = context.ServerKeyPair, &pServerSocket](const auto& pSocket) {
				pServerSocket = pSocket;
				net::VerifyClient(pSocket, serverKeyPair, [pSocket](auto, const auto&) {});
			});

			// - initiate a ping request
			std::atomic<size_t> numCallbacks(0);
			std::vector<NodePingResult> callbackResults;
			auto requestNode = test::CreateLocalHostNode(context.ServerPublicKey);
			context.pRequestor->requestPing(requestNode, [&numCallbacks, &callbackResults](auto result, const auto&) {
				callbackResults.push_back(result);
				++numCallbacks;
			});

			// Act: shutdown the requestor
			shutdown(context);
			WAIT_FOR_ONE(numCallbacks);

			// Assert:
			EXPECT_EQ(1u, callbackResults.size());
			if (NodePingResult::Failure_Timeout != callbackResults[0])
				return false;

			EXPECT_EQ(NodePingResult::Failure_Timeout, callbackResults[0]);
			return true;
		}
	}

	TEST(TEST_CLASS, RequestPingIsAsync) {
		// Arrange: non-deterministic because shutdown could be triggered during connection
		test::RunNonDeterministicTest("RequestPingIsAsync", []() {
			// Assert: controlled shutdown when requestor is destroyed
			return RunShutdownTest([](auto& context) { context.pRequestor.reset(); });
		});
	}

	TEST(TEST_CLASS, ShutdownClosesConnectedSocket) {
		// Arrange: non-deterministic because shutdown could be triggered during connection
		test::RunNonDeterministicTest("ShutdownClosesConnectedSocket", []() {
			// Assert: controlled shutdown when shutdown is explicitly called
			return RunShutdownTest([](auto& context) { context.pRequestor->shutdown(); });
		});
	}
}}
