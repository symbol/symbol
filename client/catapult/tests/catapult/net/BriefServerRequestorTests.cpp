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

#include "catapult/net/BriefServerRequestor.h"
#include "catapult/api/ChainPackets.h"
#include "catapult/api/RemoteChainApi.h"
#include "catapult/preprocessor.h"
#include "tests/test/net/BriefServerRequestorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace net {

#define TEST_CLASS BriefServerRequestorTests

	namespace {
		// region ChainStatisticsRequestor

		// use chain statistics requests to test BriefServerRequestor
		struct ChainStatisticsRequestPolicy {
			using ResponseType = api::ChainStatistics;

			static constexpr auto Friendly_Name = "chain statistics";

			static thread::future<ResponseType> CreateFuture(ionet::PacketIo& packetIo, const std::string& host) {
				// Sanity:
				EXPECT_EQ("127.0.0.1", host);

				// Act: create future
				return api::CreateRemoteChainApiWithoutRegistry(packetIo)->chainStatistics();
			}
		};

		// checker that requires chain statistics responses to have non-zero height
		class ChainStatisticsResponseCompatibilityChecker {
		public:
			constexpr bool isResponseCompatible(const ionet::Node&, const api::ChainStatistics& chainStatistics) const {
				return Height() != chainStatistics.Height;
			}
		};

		template<typename TResponseCompatibilityChecker>
		using ChainStatisticsRequestor = BriefServerRequestor<ChainStatisticsRequestPolicy, TResponseCompatibilityChecker>;

		// endregion

		// region RequestorTestContext

		template<typename TResponseCompatibilityChecker = ChainStatisticsResponseCompatibilityChecker>
		struct RequestorTestContext
				: public test::BriefServerRequestorTestContext<ChainStatisticsRequestor<TResponseCompatibilityChecker>> {
		private:
			using BaseType = test::BriefServerRequestorTestContext<ChainStatisticsRequestor<TResponseCompatibilityChecker>>;

		public:
			explicit RequestorTestContext(const utils::TimeSpan& timeout = utils::TimeSpan::FromMinutes(1))
					: BaseType(timeout, TResponseCompatibilityChecker())
			{}

		public:
			std::shared_ptr<ionet::Packet> createResponsePacket(Height height) const {
				auto pPacket = ionet::CreateSharedPacket<api::ChainStatisticsResponse>();
				pPacket->Height = height;
				return PORTABLE_MOVE(pPacket);
			}
		};

		// endregion
	}

	// region BriefServerRequestor - beginRequest

	namespace {
		using MemberBeginRequestPolicy = test::BriefServerRequestorMemberBeginRequestPolicy;

		template<typename TBeginRequestPolicy, typename TResponseCompatibilityChecker, typename TAction>
		void RunConnectedTest(
				const RequestorTestContext<TResponseCompatibilityChecker>& context,
				const std::shared_ptr<ionet::Packet>& pResponsePacket,
				TAction action) {
			// Act + Assert:
			test::RunBriefServerRequestorConnectedTest<TBeginRequestPolicy>(context, pResponsePacket, action);
		}

		template<typename TResponseCompatibilityChecker>
		void AssertFailedConnection(
				const ChainStatisticsRequestor<TResponseCompatibilityChecker>& requestor,
				const api::ChainStatistics& response) {
			// Assert:
			test::AssertBriefServerRequestorFailedConnection(requestor);
			EXPECT_EQ(Height(), response.Height);
		}
	}

	TEST(TEST_CLASS, BeginRequestFailsWhenConnectionIsNotAccepted) {
		// Arrange: create a valid packet
		RequestorTestContext<> context;
		auto pPacket = context.createResponsePacket(Height(1234));

		// Act:
		test::RunBriefServerRequestorDisconnectedTest<MemberBeginRequestPolicy>(context, [](
				const auto& requestor,
				auto result,
				const auto& response) {
			// Assert:
			EXPECT_EQ(NodeRequestResult::Failure_Connection, result);
			AssertFailedConnection(requestor, response);
		});
	}

	TEST(TEST_CLASS, BeginRequestFailsWhenConnectionInteractionFails) {
		// Arrange: create an invalid packet (no payload)
		RequestorTestContext<> context;
		auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
		pPacket->Type = api::ChainStatisticsResponse::Packet_Type;

		// Act:
		RunConnectedTest<MemberBeginRequestPolicy>(context, pPacket, [](const auto& requestor, auto result, const auto& response) {
			// Assert:
			EXPECT_EQ(NodeRequestResult::Failure_Interaction, result);
			AssertFailedConnection(requestor, response);
		});
	}

	TEST(TEST_CLASS, BeginRequestFailsWhenResponseIsIncompatible) {
		// Arrange: create a valid packet with an incompatible (0) height
		RequestorTestContext<> context;
		auto pPacket = context.createResponsePacket(Height(0));

		// Act:
		RunConnectedTest<MemberBeginRequestPolicy>(context, pPacket, [](const auto& requestor, auto result, const auto& response) {
			// Assert:
			EXPECT_EQ(NodeRequestResult::Failure_Incompatible, result);
			AssertFailedConnection(requestor, response);
		});
	}

	TEST(TEST_CLASS, BeginRequestFailsWhenResponseTimesOut) {
		// Arrange:
		RequestorTestContext<> context(utils::TimeSpan::FromMilliseconds(100));

		// Act: a nullptr packet will prevent any response from being sent
		RunConnectedTest<MemberBeginRequestPolicy>(context, nullptr, [](const auto& requestor, auto result, const auto& response) {
			// Assert:
			EXPECT_EQ(NodeRequestResult::Failure_Timeout, result);
			AssertFailedConnection(requestor, response);
		});
	}

	TEST(TEST_CLASS, BeginRequestSucceedsWhenResponseIsCompatible) {
		// Arrange:
		RequestorTestContext<> context;
		auto pPacket = context.createResponsePacket(Height(1234));

		// Act:
		RunConnectedTest<MemberBeginRequestPolicy>(context, pPacket, [](const auto& requestor, auto result, const auto& response) {
			// Assert:
			EXPECT_EQ(NodeRequestResult::Success, result);

			EXPECT_EQ(Height(1234), response.Height);

			EXPECT_EQ(1u, requestor.numTotalRequests());
			EXPECT_EQ(1u, requestor.numSuccessfulRequests());
		});
	}

	TEST(TEST_CLASS, BeginRequestDefaultCompatibilityCheckerAllowsAllResponses) {
		// Arrange: create a valid packet with an incompatible (0) height
		RequestorTestContext<detail::AlwaysCompatibleResponseCompatibilityChecker> context;
		auto pPacket = context.createResponsePacket(Height(0));

		// Act:
		RunConnectedTest<MemberBeginRequestPolicy>(context, pPacket, [](const auto& requestor, auto result, const auto& response) {
			// Assert: packet was not rejected by default compatibility checker
			EXPECT_EQ(NodeRequestResult::Success, result);

			EXPECT_EQ(Height(0), response.Height);

			EXPECT_EQ(1u, requestor.numTotalRequests());
			EXPECT_EQ(1u, requestor.numSuccessfulRequests());
		});
	}

	// endregion

	// region BriefServerRequestor - shutdown

	namespace {
		bool RunShutdownTest(const consumer<RequestorTestContext<>&>& shutdown) {
			// Arrange:
			RequestorTestContext<> context(utils::TimeSpan::FromMilliseconds(100));

			// - set up a server but don't respond to verify in order to trigger a timeout error
			test::TcpAcceptor acceptor(context.pPool->ioContext());
			std::shared_ptr<ionet::PacketSocket> pServerSocket;
			test::SpawnPacketServerWork(acceptor, [&pServerSocket](const auto& pSocket) {
				pServerSocket = pSocket;
			});

			// - initiate a request
			std::atomic<size_t> numCallbacks(0);
			std::vector<NodeRequestResult> callbackResults;
			auto requestNode = test::CreateLocalHostNode(context.ServerPublicKey);
			context.pRequestor->beginRequest(requestNode, [&numCallbacks, &callbackResults](auto result, const auto&) {
				callbackResults.push_back(result);
				++numCallbacks;
			});

			// Act: shutdown the requestor
			shutdown(context);
			WAIT_FOR_ONE(numCallbacks);

			// Assert:
			EXPECT_EQ(1u, callbackResults.size());
			if (NodeRequestResult::Failure_Timeout != callbackResults[0])
				return false;

			EXPECT_EQ(NodeRequestResult::Failure_Timeout, callbackResults[0]);
			return true;
		}
	}

	TEST(TEST_CLASS, RequestorGracefullyShutsDownWhenAsyncRequestIsOutstanding) {
		// Arrange: non-deterministic because shutdown could be triggered during connection
		test::RunNonDeterministicTest("RequestorGracefullyShutsDownWhenAsyncRequestIsOutstanding", []() {
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

	// endregion

	// region BeginRequestFuture

	namespace {
		struct BeginRequestFuturePolicy {
			template<typename TRequestor, typename TCallback = typename TRequestor::CallbackType>
			static void BeginRequest(TRequestor& requestor, const ionet::Node& requestNode, const TCallback& callback) {
				BeginRequestFuture(requestor, requestNode).then([callback](auto&& pairFuture) {
					auto pair = pairFuture.get();
					callback(pair.first, pair.second);
				});
			}
		};
	}

	TEST(TEST_CLASS, BeginRequestFutureHandlesBeginRequestFailure) {
		// Arrange: create an invalid packet (no payload)
		RequestorTestContext<> context;
		auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
		pPacket->Type = api::ChainStatisticsResponse::Packet_Type;

		// Act:
		RunConnectedTest<BeginRequestFuturePolicy>(context, pPacket, [](const auto& requestor, auto result, const auto& response) {
			// Assert:
			EXPECT_EQ(NodeRequestResult::Failure_Interaction, result);
			AssertFailedConnection(requestor, response);
		});
	}

	TEST(TEST_CLASS, BeginRequestFutureHandlesBeginRequestSuccess) {
		// Arrange:
		RequestorTestContext<> context;
		auto pPacket = context.createResponsePacket(Height(1234));

		// Act:
		RunConnectedTest<BeginRequestFuturePolicy>(context, pPacket, [](const auto& requestor, auto result, const auto& response) {
			// Assert:
			EXPECT_EQ(NodeRequestResult::Success, result);

			EXPECT_EQ(Height(1234), response.Height);

			EXPECT_EQ(1u, requestor.numTotalRequests());
			EXPECT_EQ(1u, requestor.numSuccessfulRequests());
		});
	}

	// endregion
}}
