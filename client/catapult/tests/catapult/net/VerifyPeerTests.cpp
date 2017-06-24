#include "catapult/net/VerifyPeer.h"
#include "catapult/crypto/Signer.h"
#include "catapult/ionet/PacketIo.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/Challenge.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/test/net/SocketTestUtils.h"
#include <queue>

using catapult::mocks::MockPacketIo;

namespace catapult { namespace net {

	// region VerifyClient

	namespace {
		constexpr bool Verify_Client_Key = true;
		constexpr auto Client_Private_Key = "3485d98efd7eb07adafcfd1a157d89de2796a95e780813c0258af3f5f84ed8cb";

		VerifyResult VerifyClient(const std::shared_ptr<ionet::PacketIo>& pClientIo, bool verifyClientKey = false) {
			VerifyResult result;
			Key verifiedKey;
			net::VerifyClient(
					pClientIo,
					test::GenerateKeyPair(),
					[&result, &verifiedKey](auto verifyResult, const auto& key) {
						result = verifyResult;
						verifiedKey = key;
					});

			// Assert: verified client key should be correct only for certain results
			EXPECT_EQ(verifyClientKey ? crypto::KeyPair::FromString(Client_Private_Key).publicKey() : Key(), verifiedKey);
			return result;
		}

		MockPacketIo::GenerateReadPacket CreateServerChallengeResponseGenerator(
				const std::function<void (ServerChallengeResponse&)>& modifyPacket) {
			return [modifyPacket](const auto* pPacket) {
				auto pRequest = static_cast<const ServerChallengeRequest*>(pPacket);
				auto pResponse = GenerateServerChallengeResponse(*pRequest, crypto::KeyPair::FromString(Client_Private_Key));
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

	TEST(VerifyPeerTests, VerifyClientFailsOnServerChallengeRequestWriteFailure) {
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

	TEST(VerifyPeerTests, VerifyClientFailsOnServerChallengeResponseReadFailure) {
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

	TEST(VerifyPeerTests, VerifyClientFailsOnServerChallengeResponseMalformedPacket) {
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

	TEST(VerifyPeerTests, VerifyClientFailsOnClientChallengeReponseWriteFailure) {
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

	TEST(VerifyPeerTests, VerifyClientFailsOnFailedChallenge) {
		// Arrange: queue a response with an invalid signature
		auto pMockIo = std::make_shared<MockPacketIo>();
		pMockIo->queueWrite(ionet::SocketOperationCode::Success);
		pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateServerChallengeResponseGenerator([](auto& packet) {
			packet.Signature[0] ^= 0xFF;
		}));
		pMockIo->queueWrite(ionet::SocketOperationCode::Success);

		// Act: verify
		auto result = VerifyClient(pMockIo);

		// Assert:
		EXPECT_EQ(VerifyResult::Failed_Challenge, result);
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

	TEST(VerifyPeerTests, VerifyClientSucceedsWhenResponseIsVerifiedWithoutIoErrors) {
		// Arrange: queue no errors
		auto pMockIo = CreateSuccessfulClientIo();

		// Act: verify
		auto result = VerifyClient(pMockIo, Verify_Client_Key);

		// Assert:
		EXPECT_EQ(VerifyResult::Success, result);
		EXPECT_EQ(1u, pMockIo->numReads());
		EXPECT_EQ(2u, pMockIo->numWrites());
	}

	TEST(VerifyPeerTests, VerifyClientWritesServerChallengeRequestWithNonZeroChallenge) {
		// Arrange:
		auto pMockIo = CreateSuccessfulClientIo();

		// Act: verify and retrieve the first written packet
		VerifyClient(pMockIo, Verify_Client_Key);
		const auto& packet = pMockIo->writtenPacketAt<ServerChallengeRequest>(0);

		// Assert: the challenge is non zero
		EXPECT_NE(Challenge{}, packet.Challenge);
	}

	TEST(VerifyPeerTests, VerifyClientWritesClientChallengeResponseWithValidSignature) {
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
		net::VerifyClient(pMockIo, serverKeyPair, [](auto, const auto&) {});
		const auto& packet = pMockIo->writtenPacketAt<ClientChallengeResponse>(1);

		// Assert: the signature is non zero and is verifiable
		EXPECT_NE(Signature{}, packet.Signature);
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
			Key verifiedKey;
			net::VerifyServer(
					pServerIo,
					serverKeyPair.publicKey(),
					clientKeyPair,
					[&result, &verifiedKey](auto verifyResult, const auto& key) {
						result = verifyResult;
						verifiedKey = key;
					});

			// Assert: verified server key should be correct for all results
			EXPECT_EQ(serverKeyPair.publicKey(), verifiedKey);
			return result;
		}

		VerifyResult VerifyServer(const std::shared_ptr<ionet::PacketIo>& pServerIo) {
			return VerifyServer(test::GenerateKeyPair(), test::GenerateKeyPair(), pServerIo);
		}

		MockPacketIo::GenerateReadPacket CreateServerChallengeRequestGenerator(
				const std::function<void (ionet::Packet&)>& modifyPacket) {
			return [modifyPacket](const auto*) {
				auto pRequest = GenerateServerChallengeRequest();
				modifyPacket(*pRequest);
				return pRequest;
			};
		}

		MockPacketIo::GenerateReadPacket CreateServerChallengeRequestGenerator() {
			return CreateServerChallengeRequestGenerator([](const auto&) {});
		}

		MockPacketIo::GenerateReadPacket CreateClientChallengeResponseGenerator(
				const std::function<void (ClientChallengeResponse&)>& modifyPacket) {
			return [modifyPacket](const auto* pPacket) {
				auto pRequest = static_cast<const ClientChallengeRequest*>(pPacket);
				auto pResponse = GenerateClientChallengeResponse(*pRequest, test::GenerateKeyPair());
				modifyPacket(*pResponse);
				return pResponse;
			};
		}

		MockPacketIo::GenerateReadPacket CreateClientChallengeResponseGenerator(
				const crypto::KeyPair& serverKeyPair) {
			return [&serverKeyPair](const auto* pPacket) {
				auto pRequest = static_cast<const ClientChallengeRequest*>(pPacket);
				return GenerateClientChallengeResponse(*pRequest, serverKeyPair);
			};
		}
	}

	TEST(VerifyPeerTests, VerifyServerFailsOnServerChallengeRequestReadFailure) {
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

	TEST(VerifyPeerTests, VerifyServerFailsOnServerChallengeRequestMalformedPacket) {
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

	TEST(VerifyPeerTests, VerifyServerFailsOnServerChallengeResponseWriteFailure) {
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

	TEST(VerifyPeerTests, VerifyServerFailsOnClientChallengeResponseReadFailure) {
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

	TEST(VerifyPeerTests, VerifyServerFailsOnClientChallengeResponseMalformedPacket) {
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

	TEST(VerifyPeerTests, VerifyServerFailsOnFailedChallenge) {
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
		EXPECT_EQ(VerifyResult::Failed_Challenge, result);
		EXPECT_EQ(2u, pMockIo->numReads());
		EXPECT_EQ(1u, pMockIo->numWrites());
	}

	namespace {
		std::shared_ptr<MockPacketIo> CreateSuccessfulServerIo(const crypto::KeyPair& serverKeyPair) {
			auto pMockIo = std::make_shared<MockPacketIo>();
			pMockIo->queueRead(ionet::SocketOperationCode::Success, CreateServerChallengeRequestGenerator());
			pMockIo->queueWrite(ionet::SocketOperationCode::Success);
			pMockIo->queueRead(
					ionet::SocketOperationCode::Success,
					CreateClientChallengeResponseGenerator(serverKeyPair));
			return pMockIo;
		}
	}

	TEST(VerifyPeerTests, VerifyServerSucceedsWhenResponseIsVerifiedWithoutIoErrors) {
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

	TEST(VerifyPeerTests, VerifyServerWritesServerChallengeRequestWithNonZeroChallenge) {
		// Arrange:
		auto serverKeyPair = test::GenerateKeyPair();
		auto clientKeyPair = test::GenerateKeyPair();
		auto pMockIo = CreateSuccessfulServerIo(serverKeyPair);

		// Act: verify and retrieve the first written packet
		VerifyServer(serverKeyPair, clientKeyPair, pMockIo);
		const auto& packet = pMockIo->writtenPacketAt<ServerChallengeResponse>(0);

		// Assert: the challenge is non zero
		EXPECT_NE(Challenge{}, packet.Challenge);
	}

	TEST(VerifyPeerTests, VerifyServerWritesClientChallengeResponseWithValidSignature) {
		// Arrange:
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
		pMockIo->queueRead(
				ionet::SocketOperationCode::Success,
				CreateClientChallengeResponseGenerator(serverKeyPair));

		// Act: verify and retrieve the first written packet
		VerifyServer(serverKeyPair, clientKeyPair, pMockIo);
		const auto& packet = pMockIo->writtenPacketAt<ServerChallengeResponse>(0);

		// Assert: the signature is non zero and is verifiable
		EXPECT_NE(Signature{}, packet.Signature);
		EXPECT_TRUE(crypto::Verify(clientKeyPair.publicKey(), challenge, packet.Signature));
	}

	// endregion

	// region VerifyClient / VerifyServer Handshake

	TEST(VerifyPeerTests, VerifyClientAndVerifyServerCanMutuallyValidate) {
		// Arrange:
		auto serverKeyPair = test::GenerateKeyPair();
		auto clientKeyPair = test::GenerateKeyPair();
		auto pPool = test::CreateStartedIoServiceThreadPool();
		auto& service = pPool->service();
		std::atomic<size_t> numResults(0);

		// Act: start a server and client verify operation
		VerifyResult serverResult;
		Key verifiedClientPublicKey;
		test::SpawnPacketServerWork(service, [&](const auto& pSocket) -> void {
			VerifyClient(pSocket, serverKeyPair, [&](auto result, const auto& key) {
				serverResult = result;
				verifiedClientPublicKey = key;
				++numResults;
			});
		});

		VerifyResult clientResult;
		Key verifiedServerPublicKey;
		test::SpawnPacketClientWork(service, [&](const auto& pSocket) -> void {
			VerifyServer(pSocket, serverKeyPair.publicKey(), clientKeyPair, [&](auto result, const auto& key) {
				clientResult = result;
				verifiedServerPublicKey = key;
				++numResults;
			});
		});

		// - wait for both verifications to complete
		WAIT_FOR_VALUE(numResults, 2u);

		// Assert: both verifications succeeded
		EXPECT_EQ(VerifyResult::Success, serverResult);
		EXPECT_EQ(clientKeyPair.publicKey(), verifiedClientPublicKey);

		EXPECT_EQ(VerifyResult::Success, clientResult);
		EXPECT_EQ(serverKeyPair.publicKey(), verifiedServerPublicKey);
	}

	// endregion
}}
