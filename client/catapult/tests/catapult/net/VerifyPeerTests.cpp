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

#include "catapult/net/VerifyPeer.h"
#include "catapult/crypto/Signer.h"
#include "catapult/ionet/PacketIo.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/Challenge.h"
#include "catapult/thread/IoThreadPool.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/test/net/SocketTestUtils.h"
#include <queue>

using catapult::mocks::MockPacketIo;

namespace catapult { namespace net {

#define TEST_CLASS VerifyPeerTests

	// region VerifyClient

	namespace {
		constexpr bool Verify_Client_Key = true;
		constexpr auto Client_Private_Key = "3485D98EFD7EB07ADAFCFD1A157D89DE2796A95E780813C0258AF3F5F84ED8CB";
		constexpr auto Default_Security_Mode = ionet::ConnectionSecurityMode::Signed;
		constexpr auto Default_Allowed_Security_Mode_Mask = static_cast<ionet::ConnectionSecurityMode>(0xA);

		VerifyResult VerifyClient(const std::shared_ptr<ionet::PacketIo>& pClientIo, const VerifiedPeerInfo& expectedPeerInfo) {
			VerifyResult result;
			VerifiedPeerInfo verifiedPeerInfo;
			net::VerifyClient(pClientIo, test::GenerateKeyPair(), Default_Allowed_Security_Mode_Mask, [&result, &verifiedPeerInfo](
					auto verifyResult,
					const auto& peerInfo) {
				result = verifyResult;
				verifiedPeerInfo = peerInfo;
			});

			// Assert:
			EXPECT_EQ(expectedPeerInfo.PublicKey, verifiedPeerInfo.PublicKey);
			EXPECT_EQ(expectedPeerInfo.SecurityMode, verifiedPeerInfo.SecurityMode);
			return result;
		}

		VerifyResult VerifyClient(const std::shared_ptr<ionet::PacketIo>& pClientIo, bool shouldExpectPeerInfo = false) {
			// Assert: verified client key should be correct only for certain results
			auto expectedPeerInfo = shouldExpectPeerInfo
					? VerifiedPeerInfo{ crypto::KeyPair::FromString(Client_Private_Key).publicKey(), Default_Security_Mode }
					: VerifiedPeerInfo{ Key(), static_cast<ionet::ConnectionSecurityMode>(0) };

			return VerifyClient(pClientIo, expectedPeerInfo);
		}

		MockPacketIo::GenerateReadPacket CreateServerChallengeResponseGenerator(const consumer<ServerChallengeResponse&>& modifyPacket) {
			return [modifyPacket](const auto* pPacket) {
				auto pRequest = static_cast<const ServerChallengeRequest*>(pPacket);
				auto pResponse = GenerateServerChallengeResponse(
						*pRequest,
						crypto::KeyPair::FromString(Client_Private_Key),
						Default_Security_Mode);
				modifyPacket(*pResponse);
				return pResponse;
			};
		}

		MockPacketIo::GenerateReadPacket CreateServerChallengeResponseGenerator() {
			return CreateServerChallengeResponseGenerator([](const auto&) {});
		}

		template<typename TAssertHandler>
		void AssertMalformedPacketHandling(TAssertHandler assertHandler) {
			// - unexpected size
			assertHandler([](auto& packet) { ++packet.Size; });
			// - unexpected type
			assertHandler([](auto& packet) { packet.Type = ionet::PacketType::Undefined; });
		}
	}

	TEST(TEST_CLASS, VerifyClientFailsOnServerChallengeRequestWriteFailure) {
		// Arrange: queue a write error
		auto pMockIo = std::make_shared<MockPacketIo>();
		pMockIo->queueWrite(ionet::SocketOperationCode::Write_Error);

		// Act: verify
		auto result = VerifyClient(pMockIo);

		// Assert:
		EXPECT_EQ(VerifyResult::Io_Error_ServerChallengeRequest, result);
		EXPECT_EQ(0u, pMockIo->numReads());
		EXPECT_EQ(1u, pMockIo->numWrites());
	}

	TEST(TEST_CLASS, VerifyClientFailsOnServerChallengeResponseReadFailure) {
		// Arrange: queue a read error
		auto pMockIo = std::make_shared<MockPacketIo>();
		pMockIo->queueWrite(ionet::SocketOperationCode::Success);
		pMockIo->queueRead(ionet::SocketOperationCode::Read_Error);

		// Act: verify
		auto result = VerifyClient(pMockIo);

		// Assert:
		EXPECT_EQ(VerifyResult::Io_Error_ServerChallengeResponse, result);
		EXPECT_EQ(1u, pMockIo->numReads());
		EXPECT_EQ(1u, pMockIo->numWrites());
	}

	TEST(TEST_CLASS, VerifyClientFailsOnServerChallengeResponseMalformedPacket) {
		AssertMalformedPacketHandling([](const auto& modifyPacket) {
			// Arrange: queue a malformed packet
			auto pMockIo = std::make_shared<MockPacketIo>();
			pMockIo->queueWrite(ionet::SocketOperationCode::Success);
			pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateServerChallengeResponseGenerator(modifyPacket));

			// Act: verify
			auto result = VerifyClient(pMockIo);

			// Assert:
			EXPECT_EQ(VerifyResult::Malformed_Data, result);
			EXPECT_EQ(1u, pMockIo->numReads());
			EXPECT_EQ(1u, pMockIo->numWrites());
		});
	}

	TEST(TEST_CLASS, VerifyClientFailsOnClientChallengeReponseWriteFailure) {
		// Arrange: queue a write error on the second write
		auto pMockIo = std::make_shared<MockPacketIo>();
		pMockIo->queueWrite(ionet::SocketOperationCode::Success);
		pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateServerChallengeResponseGenerator());
		pMockIo->queueWrite(ionet::SocketOperationCode::Write_Error);

		// Act: verify
		auto result = VerifyClient(pMockIo, Verify_Client_Key);

		// Assert:
		EXPECT_EQ(VerifyResult::Io_Error_ClientChallengeResponse, result);
		EXPECT_EQ(1u, pMockIo->numReads());
		EXPECT_EQ(2u, pMockIo->numWrites());
	}

	namespace {
		void AssertVerifyClientFailsOnInvalidSecurityMode(ionet::ConnectionSecurityMode securityMode) {
			// Arrange: queue a response with an invalid security mode
			auto pMockIo = std::make_shared<MockPacketIo>();
			pMockIo->queueWrite(ionet::SocketOperationCode::Success);
			pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateServerChallengeResponseGenerator([securityMode](auto& packet) {
				packet.SecurityMode = securityMode;
			}));
			pMockIo->queueWrite(ionet::SocketOperationCode::Success);

			// Act: verify
			auto clientPublicKey = crypto::KeyPair::FromString(Client_Private_Key).publicKey();
			auto result = VerifyClient(pMockIo, { clientPublicKey, securityMode });

			// Assert:
			EXPECT_EQ(VerifyResult::Failure_Unsupported_Connection, result);
			EXPECT_EQ(1u, pMockIo->numReads());
			EXPECT_EQ(1u, pMockIo->numWrites());
		}
	}

	TEST(TEST_CLASS, VerifyClientFailsOnInvalidSecurityMode) {
		AssertVerifyClientFailsOnInvalidSecurityMode(static_cast<ionet::ConnectionSecurityMode>(4));
	}

	TEST(TEST_CLASS, VerifyClientFailsOnMultipleSecurityModes) {
		AssertVerifyClientFailsOnInvalidSecurityMode(Default_Allowed_Security_Mode_Mask);
	}

	TEST(TEST_CLASS, VerifyClientFailsOnFailedChallenge) {
		// Arrange: queue a response with an invalid signature
		auto pMockIo = std::make_shared<MockPacketIo>();
		pMockIo->queueWrite(ionet::SocketOperationCode::Success);
		pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateServerChallengeResponseGenerator([](auto& packet) {
			packet.Signature[0] ^= 0xFF;
		}));
		pMockIo->queueWrite(ionet::SocketOperationCode::Success);

		// Act: verify
		auto result = VerifyClient(pMockIo, Verify_Client_Key);

		// Assert:
		EXPECT_EQ(VerifyResult::Failure_Challenge, result);
		EXPECT_EQ(1u, pMockIo->numReads());
		EXPECT_EQ(1u, pMockIo->numWrites());
	}

	namespace {
		std::shared_ptr<MockPacketIo> CreateSuccessfulClientIo() {
			auto pMockIo = std::make_shared<MockPacketIo>();
			pMockIo->queueWrite(ionet::SocketOperationCode::Success);
			pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateServerChallengeResponseGenerator());
			pMockIo->queueWrite(ionet::SocketOperationCode::Success);
			return pMockIo;
		}
	}

	TEST(TEST_CLASS, VerifyClientSucceedsWhenResponseIsVerifiedWithoutIoErrors) {
		// Arrange: queue no errors
		auto pMockIo = CreateSuccessfulClientIo();

		// Act: verify
		auto result = VerifyClient(pMockIo, Verify_Client_Key);

		// Assert:
		EXPECT_EQ(VerifyResult::Success, result);
		EXPECT_EQ(1u, pMockIo->numReads());
		EXPECT_EQ(2u, pMockIo->numWrites());
	}

	TEST(TEST_CLASS, VerifyClientWritesServerChallengeRequestWithNonzeroChallenge) {
		// Arrange:
		auto pMockIo = CreateSuccessfulClientIo();

		// Act: verify and retrieve the first written packet
		VerifyClient(pMockIo, Verify_Client_Key);
		const auto& packet = pMockIo->writtenPacketAt<ServerChallengeRequest>(0);

		// Assert: the challenge is nonzero
		EXPECT_NE(Challenge(), packet.Challenge);
	}

	TEST(TEST_CLASS, VerifyClientWritesClientChallengeResponseWithValidSignature) {
		// Arrange: queue no errors
		Challenge challenge;
		auto serverKeyPair = test::GenerateKeyPair();
		auto pMockIo = std::make_shared<MockPacketIo>();
		pMockIo->queueWrite(ionet::SocketOperationCode::Success);
		pMockIo->queueRead(ionet::SocketOperationCode::Success, [&challenge](const auto* pPacket) {
			auto pResponse = CreateServerChallengeResponseGenerator()(pPacket);
			challenge = static_cast<const ServerChallengeResponse&>(*pResponse).Challenge;
			return pResponse;
		});
		pMockIo->queueWrite(ionet::SocketOperationCode::Success);

		// Act: verify and retreive the second written packet
		net::VerifyClient(pMockIo, serverKeyPair, Default_Allowed_Security_Mode_Mask, [](auto, const auto&) {});
		const auto& packet = pMockIo->writtenPacketAt<ClientChallengeResponse>(1);

		// Assert: the signature is nonzero and is verifiable
		EXPECT_NE(Signature(), packet.Signature);
		EXPECT_TRUE(crypto::Verify(serverKeyPair.publicKey(), challenge, packet.Signature));
	}

	// endregion

	// region VerifyServer

	namespace {
		VerifyResult VerifyServer(
				const crypto::KeyPair& serverKeyPair,
				const crypto::KeyPair& clientKeyPair,
				const std::shared_ptr<ionet::PacketIo>& pServerIo) {
			VerifyResult result;
			VerifiedPeerInfo verifiedPeerInfo;
			auto serverPeerInfo = VerifiedPeerInfo{ serverKeyPair.publicKey(), Default_Security_Mode };
			net::VerifyServer(pServerIo, serverPeerInfo, clientKeyPair, [&result, &verifiedPeerInfo](
					auto verifyResult,
					const auto& peerInfo) {
				result = verifyResult;
				verifiedPeerInfo = peerInfo;
			});

			// Assert: verified server key should be correct for all results
			EXPECT_EQ(serverKeyPair.publicKey(), verifiedPeerInfo.PublicKey);
			EXPECT_EQ(Default_Security_Mode, verifiedPeerInfo.SecurityMode);
			return result;
		}

		VerifyResult VerifyServer(const std::shared_ptr<ionet::PacketIo>& pServerIo) {
			return VerifyServer(test::GenerateKeyPair(), test::GenerateKeyPair(), pServerIo);
		}

		MockPacketIo::GenerateReadPacket CreateServerChallengeRequestGenerator(const consumer<ionet::Packet&>& modifyPacket) {
			return [modifyPacket](const auto*) {
				auto pRequest = GenerateServerChallengeRequest();
				modifyPacket(*pRequest);
				return pRequest;
			};
		}

		MockPacketIo::GenerateReadPacket CreateServerChallengeRequestGenerator() {
			return CreateServerChallengeRequestGenerator([](const auto&) {});
		}

		MockPacketIo::GenerateReadPacket CreateClientChallengeResponseGenerator(const consumer<ClientChallengeResponse&>& modifyPacket) {
			return [modifyPacket](const auto* pPacket) {
				auto pRequest = static_cast<const ServerChallengeResponse*>(pPacket);
				auto pResponse = GenerateClientChallengeResponse(*pRequest, test::GenerateKeyPair());
				modifyPacket(*pResponse);
				return pResponse;
			};
		}

		MockPacketIo::GenerateReadPacket CreateClientChallengeResponseGenerator(const crypto::KeyPair& serverKeyPair) {
			return [&serverKeyPair](const auto* pPacket) {
				auto pRequest = static_cast<const ServerChallengeResponse*>(pPacket);
				return GenerateClientChallengeResponse(*pRequest, serverKeyPair);
			};
		}
	}

	TEST(TEST_CLASS, VerifyServerFailsOnServerChallengeRequestReadFailure) {
		// Arrange: queue a read error
		auto pMockIo = std::make_shared<MockPacketIo>();
		pMockIo->queueRead(ionet::SocketOperationCode::Read_Error);

		// Act: verify
		auto result = VerifyServer(pMockIo);

		// Assert:
		EXPECT_EQ(VerifyResult::Io_Error_ServerChallengeRequest, result);
		EXPECT_EQ(1u, pMockIo->numReads());
		EXPECT_EQ(0u, pMockIo->numWrites());
	}

	TEST(TEST_CLASS, VerifyServerFailsOnServerChallengeRequestMalformedPacket) {
		AssertMalformedPacketHandling([](const auto& modifyPacket) {
			// Arrange: queue a malformed packet
			auto pMockIo = std::make_shared<MockPacketIo>();
			pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateServerChallengeRequestGenerator(modifyPacket));

			// Act: verify
			auto result = VerifyServer(pMockIo);

			// Assert:
			EXPECT_EQ(VerifyResult::Malformed_Data, result);
			EXPECT_EQ(1u, pMockIo->numReads());
			EXPECT_EQ(0u, pMockIo->numWrites());
		});
	}

	TEST(TEST_CLASS, VerifyServerFailsOnServerChallengeResponseWriteFailure) {
		// Arrange: queue a write error
		auto pMockIo = std::make_shared<MockPacketIo>();
		pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateServerChallengeRequestGenerator());
		pMockIo->queueWrite(ionet::SocketOperationCode::Write_Error);

		// Act: verify
		auto result = VerifyServer(pMockIo);

		// Assert:
		EXPECT_EQ(VerifyResult::Io_Error_ServerChallengeResponse, result);
		EXPECT_EQ(1u, pMockIo->numReads());
		EXPECT_EQ(1u, pMockIo->numWrites());
	}

	TEST(TEST_CLASS, VerifyServerFailsOnClientChallengeResponseReadFailure) {
		// Arrange: queue a read error
		auto pMockIo = std::make_shared<MockPacketIo>();
		pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateServerChallengeRequestGenerator());
		pMockIo->queueWrite(ionet::SocketOperationCode::Success);
		pMockIo->queueRead(ionet::SocketOperationCode::Read_Error);

		// Act: verify
		auto result = VerifyServer(pMockIo);

		// Assert:
		EXPECT_EQ(VerifyResult::Io_Error_ClientChallengeResponse, result);
		EXPECT_EQ(2u, pMockIo->numReads());
		EXPECT_EQ(1u, pMockIo->numWrites());
	}

	TEST(TEST_CLASS, VerifyServerFailsOnClientChallengeResponseMalformedPacket) {
		AssertMalformedPacketHandling([](const auto& modifyPacket) {
			// Arrange: queue a malformed packet
			auto pMockIo = std::make_shared<MockPacketIo>();
			pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateServerChallengeRequestGenerator());
			pMockIo->queueWrite(ionet::SocketOperationCode::Success);
			pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateClientChallengeResponseGenerator(modifyPacket));

			// Act: verify
			auto result = VerifyServer(pMockIo);

			// Assert:
			EXPECT_EQ(VerifyResult::Malformed_Data, result);
			EXPECT_EQ(2u, pMockIo->numReads());
			EXPECT_EQ(1u, pMockIo->numWrites());
		});
	}

	TEST(TEST_CLASS, VerifyServerFailsOnFailedChallenge) {
		// Arrange: queue a response with an invalid signature
		auto pMockIo = std::make_shared<MockPacketIo>();
		pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateServerChallengeRequestGenerator());
		pMockIo->queueWrite(ionet::SocketOperationCode::Success);
		pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateClientChallengeResponseGenerator([](auto& packet) {
			packet.Signature[0] ^= 0xFF;
		}));

		// Act: verify
		auto result = VerifyServer(pMockIo);

		// Assert:
		EXPECT_EQ(VerifyResult::Failure_Challenge, result);
		EXPECT_EQ(2u, pMockIo->numReads());
		EXPECT_EQ(1u, pMockIo->numWrites());
	}

	namespace {
		std::shared_ptr<MockPacketIo> CreateSuccessfulServerIo(const crypto::KeyPair& serverKeyPair) {
			auto pMockIo = std::make_shared<MockPacketIo>();
			pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateServerChallengeRequestGenerator());
			pMockIo->queueWrite(ionet::SocketOperationCode::Success);
			pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateClientChallengeResponseGenerator(serverKeyPair));
			return pMockIo;
		}
	}

	TEST(TEST_CLASS, VerifyServerSucceedsWhenResponseIsVerifiedWithoutIoErrors) {
		// Arrange: queue no errors
		auto serverKeyPair = test::GenerateKeyPair();
		auto clientKeyPair = test::GenerateKeyPair();
		auto pMockIo = CreateSuccessfulServerIo(serverKeyPair);

		// Act: verify
		auto result = VerifyServer(serverKeyPair, clientKeyPair, pMockIo);

		// Assert:
		EXPECT_EQ(VerifyResult::Success, result);
		EXPECT_EQ(2u, pMockIo->numReads());
		EXPECT_EQ(1u, pMockIo->numWrites());
	}

	TEST(TEST_CLASS, VerifyServerWritesServerChallengeRequestWithNonzeroChallenge) {
		// Arrange:
		auto serverKeyPair = test::GenerateKeyPair();
		auto clientKeyPair = test::GenerateKeyPair();
		auto pMockIo = CreateSuccessfulServerIo(serverKeyPair);

		// Act: verify and retrieve the first written packet
		VerifyServer(serverKeyPair, clientKeyPair, pMockIo);
		const auto& packet = pMockIo->writtenPacketAt<ServerChallengeResponse>(0);

		// Assert: the challenge is nonzero
		EXPECT_NE(Challenge(), packet.Challenge);
	}

	TEST(TEST_CLASS, VerifyServerWritesClientChallengeResponseWithValidSignature) {
		// Arrange:
		constexpr auto Challenge_Size = std::tuple_size_v<Challenge>;
		auto serverKeyPair = test::GenerateKeyPair();
		auto clientKeyPair = test::GenerateKeyPair();
		Challenge challenge;
		auto pMockIo = std::make_shared<MockPacketIo>();
		pMockIo->queueRead(ionet::SocketOperationCode::Success, [&challenge](const auto* pPacket) {
			auto pRequest = CreateServerChallengeRequestGenerator()(pPacket);
			challenge = static_cast<const ServerChallengeResponse&>(*pRequest).Challenge;
			return pRequest;
		});
		pMockIo->queueWrite(ionet::SocketOperationCode::Success);
		pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateClientChallengeResponseGenerator(serverKeyPair));

		// Act: verify and retrieve the first written packet
		VerifyServer(serverKeyPair, clientKeyPair, pMockIo);
		const auto& packet = pMockIo->writtenPacketAt<ServerChallengeResponse>(0);

		// - construct expected signed data
		auto signedData = std::vector<uint8_t>(Challenge_Size + 1);
		std::memcpy(signedData.data(), challenge.data(), Challenge_Size);
		signedData[Challenge_Size] = utils::to_underlying_type(Default_Security_Mode);

		// Assert: the signature is nonzero and is verifiable
		EXPECT_NE(Signature(), packet.Signature);
		EXPECT_TRUE(crypto::Verify(clientKeyPair.publicKey(), signedData, packet.Signature));
	}

	// endregion

	// region VerifyClient / VerifyServer Handshake

	namespace {
		void AssertVerifyClientAndVerifyServerCanMutuallyValidate(
				ionet::ConnectionSecurityMode securityMode,
				ionet::ConnectionSecurityMode allowedSecurityModes) {
			// Arrange:
			auto serverKeyPair = test::GenerateKeyPair();
			auto clientKeyPair = test::GenerateKeyPair();
			auto pPool = test::CreateStartedIoThreadPool();
			auto& ioContext = pPool->ioContext();
			std::atomic<size_t> numResults(0);

			// Act: start a server and client verify operation
			VerifyResult serverResult;
			VerifiedPeerInfo verifiedClientPeerInfo;
			test::SpawnPacketServerWork(ioContext, [&](const auto& pSocket) {
				net::VerifyClient(pSocket, serverKeyPair, allowedSecurityModes, [&](auto result, const auto& peerInfo) {
					serverResult = result;
					verifiedClientPeerInfo = peerInfo;
					++numResults;
				});
			});

			VerifyResult clientResult;
			VerifiedPeerInfo verifiedServerPeerInfo;
			test::SpawnPacketClientWork(ioContext, [&](const auto& pSocket) {
				auto severPeerInfo = VerifiedPeerInfo{ serverKeyPair.publicKey(), securityMode };
				net::VerifyServer(pSocket, severPeerInfo, clientKeyPair, [&](auto result, const auto& peerInfo) {
					clientResult = result;
					verifiedServerPeerInfo = peerInfo;
					++numResults;
				});
			});

			// - wait for both verifications to complete
			WAIT_FOR_VALUE(2u, numResults);

			// Assert: both verifications succeeded
			EXPECT_EQ(VerifyResult::Success, serverResult);
			EXPECT_EQ(clientKeyPair.publicKey(), verifiedClientPeerInfo.PublicKey);
			EXPECT_EQ(securityMode, verifiedClientPeerInfo.SecurityMode);

			EXPECT_EQ(VerifyResult::Success, clientResult);
			EXPECT_EQ(serverKeyPair.publicKey(), verifiedServerPeerInfo.PublicKey);
			EXPECT_EQ(securityMode, verifiedServerPeerInfo.SecurityMode);
		}
	}

	TEST(TEST_CLASS, VerifyClientAndVerifyServerCanMutuallyValidate_ExactMatch) {
		AssertVerifyClientAndVerifyServerCanMutuallyValidate(ionet::ConnectionSecurityMode::Signed, ionet::ConnectionSecurityMode::Signed);
	}

	TEST(TEST_CLASS, VerifyClientAndVerifyServerCanMutuallyValidate_Subset) {
		AssertVerifyClientAndVerifyServerCanMutuallyValidate(ionet::ConnectionSecurityMode::Signed, Default_Allowed_Security_Mode_Mask);
	}

	// endregion
}}
