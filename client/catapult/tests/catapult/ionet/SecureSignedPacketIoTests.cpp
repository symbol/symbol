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

#include "catapult/ionet/SecureSignedPacketIo.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/Signer.h"
#include "catapult/ionet/PacketPayloadFactory.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/PacketIoTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS SecureSignedPacketIoTests

	namespace {
		struct TestContext {
		public:
			explicit TestContext(uint32_t maxSignedPacketDataSize = std::numeric_limits<uint32_t>::max())
					: pMockPacketIo(std::make_shared<mocks::MockPacketIo>())
					, KeyPair(test::GenerateKeyPair())
					, RemoteKeyPair(test::GenerateKeyPair())
					, RemoteKey(RemoteKeyPair.publicKey())
					, pSecureIo(CreateSecureSignedPacketIo(pMockPacketIo, KeyPair, RemoteKey, maxSignedPacketDataSize))
					, pSecureBatchReader(CreateSecureSignedBatchPacketReader(pMockPacketIo, RemoteKey))
			{}

		public:
			std::shared_ptr<mocks::MockPacketIo> pMockPacketIo;
			crypto::KeyPair KeyPair;
			crypto::KeyPair RemoteKeyPair;
			Key RemoteKey;
			std::shared_ptr<PacketIo> pSecureIo;
			std::shared_ptr<BatchPacketReader> pSecureBatchReader;
		};

		Signature SignPacket(const crypto::KeyPair& keyPair, const Packet& packet) {
			Hash256 packetHash;
			crypto::Sha3_256({ reinterpret_cast<const uint8_t*>(&packet), packet.Size }, packetHash);

			Signature signature;
			crypto::Sign(keyPair, packetHash, signature);
			return signature;
		}
	}

	// region PacketIo - write

	namespace {
		template<typename TAction>
		void RunWritePayloadTest(
				TestContext&& context,
				const std::vector<std::shared_ptr<model::VerifiableEntity>>& entities,
				uint32_t numEntitiesBytes,
				TAction action) {
			// Arrange:
			context.pMockPacketIo->queueWrite(SocketOperationCode::Success);

			auto payload = PacketPayloadFactory::FromEntities(PacketType::Push_Transactions, entities);

			// Act:
			SocketOperationCode writeCode;
			context.pSecureIo->write(payload, [&writeCode](auto code) {
				writeCode = code;
			});

			const auto& writtenPacket = context.pMockPacketIo->writtenPacketAt<Packet>(0);

			// Assert:
			EXPECT_EQ(SocketOperationCode::Success, writeCode);

			ASSERT_EQ(sizeof(PacketHeader) + sizeof(Signature) + sizeof(PacketHeader) + numEntitiesBytes, writtenPacket.Size);
			EXPECT_EQ(PacketType::Secure_Signed, writtenPacket.Type);

			const auto& signature = reinterpret_cast<const Signature&>(*(&writtenPacket + 1));
			const auto& childPacket = reinterpret_cast<const Packet&>(*(reinterpret_cast<const uint8_t*>(&signature) + Signature_Size));
			ASSERT_EQ(sizeof(PacketHeader) + numEntitiesBytes, childPacket.Size);
			EXPECT_EQ(PacketType::Push_Transactions, childPacket.Type);

			EXPECT_EQ(SignPacket(context.KeyPair, childPacket), signature);

			action(childPacket);
		}
	}

	TEST(TEST_CLASS, WriteSignsPayloadWithNoBuffers) {
		// Act:
		RunWritePayloadTest(TestContext(), {}, 0, [](const auto&) {});
	}

	TEST(TEST_CLASS, WriteSignsPayloadWithSingleBuffer) {
		// Arrange:
		auto entities = std::vector<std::shared_ptr<model::VerifiableEntity>>{ test::CreateRandomEntityWithSize<>(126) };

		// Act:
		RunWritePayloadTest(TestContext(), entities, 126, [&entities](const auto& childPacket) {
			// Assert:
			EXPECT_EQ_MEMORY(entities[0].get(), childPacket.Data(), entities[0]->Size);
		});
	}

	TEST(TEST_CLASS, WriteSignsPayloadWithMultipleBuffers) {
		// Arrange:
		auto entities = std::vector<std::shared_ptr<model::VerifiableEntity>>{
			test::CreateRandomEntityWithSize<>(126),
			test::CreateRandomEntityWithSize<>(212),
			test::CreateRandomEntityWithSize<>(134),
		};

		// Act:
		RunWritePayloadTest(TestContext(), entities, 126 + 212 + 134, [&entities](const auto& childPacket) {
			// Assert:
			EXPECT_EQ_MEMORY(entities[0].get(), childPacket.Data(), entities[0]->Size);
			EXPECT_EQ_MEMORY(entities[1].get(), childPacket.Data() + 126, entities[1]->Size);
			EXPECT_EQ_MEMORY(entities[2].get(), childPacket.Data() + 126 + 212, entities[2]->Size);
		});
	}

	TEST(TEST_CLASS, WriteForwardsInnerWriteError) {
		// Arrange: set a write error
		TestContext context;
		context.pMockPacketIo->queueWrite(SocketOperationCode::Write_Error);

		auto entities = std::vector<std::shared_ptr<model::VerifiableEntity>>{ test::CreateRandomEntityWithSize<>(126) };
		auto payload = PacketPayloadFactory::FromEntities(PacketType::Push_Transactions, entities);

		// Act:
		SocketOperationCode writeCode;
		context.pSecureIo->write(payload, [&writeCode](auto code) {
			writeCode = code;
		});

		// Assert:
		EXPECT_EQ(SocketOperationCode::Write_Error, writeCode);
	}

	namespace {
		void AssertMalformedDataWrite(TestContext&& context, const PacketPayload& payload) {
			// Arrange:
			context.pMockPacketIo->queueWrite(SocketOperationCode::Success);

			// Act:
			SocketOperationCode writeCode;
			context.pSecureIo->write(payload, [&writeCode](auto code) {
				writeCode = code;
			});

			// Assert:
			EXPECT_EQ(SocketOperationCode::Malformed_Data, writeCode);
		}
	}

	TEST(TEST_CLASS, WriteFailsWhenPacketPayloadIsUnset) {
		// Arrange:
		AssertMalformedDataWrite(TestContext(), PacketPayload());
	}

	TEST(TEST_CLASS, WriteFailsWhenPacketPayloadExceedsMaxPacketDataSize) {
		// Arrange:
		auto entities = std::vector<std::shared_ptr<model::VerifiableEntity>>{ test::CreateRandomEntityWithSize<>(126) };
		auto payload = PacketPayloadFactory::FromEntities(PacketType::Push_Transactions, entities);

		// Assert:
		AssertMalformedDataWrite(TestContext(126 - 1), payload);
	}

	TEST(TEST_CLASS, WriteSucceedsWhenPacketPayloadIsExactlyMaxPacketDataSize) {
		// Arrange: notice that maxSignedPacketDataSize only applies to the inner packet, the outer packet size can exceed it
		auto entities = std::vector<std::shared_ptr<model::VerifiableEntity>>{ test::CreateRandomEntityWithSize<>(126) };

		// Act:
		RunWritePayloadTest(TestContext(126), entities, 126, [&entities](const auto& childPacket) {
			// Assert:
			EXPECT_EQ_MEMORY(entities[0].get(), childPacket.Data(), entities[0]->Size);

			// Sanity:
			ASSERT_EQ(sizeof(PacketHeader) + 126, childPacket.Size);
		});
	}

	// endregion

	// region PacketIo - read, BatchPacketReader - readMultiple (single packet)

	namespace {
		// note: GetSecureSigned* helpers assume a secure signed packet

		Signature& GetSecureSignedSignature(Packet& packet) {
			return reinterpret_cast<Signature&>(*(&packet + 1));
		}

		Packet& GetSecureSignedChildPacket(Packet& packet) {
			auto& signature = GetSecureSignedSignature(packet);
			return reinterpret_cast<Packet&>(*(reinterpret_cast<uint8_t*>(&signature) + Signature_Size));
		}

		std::shared_ptr<Packet> CreateSecureSignedPacket(const crypto::KeyPair& keyPair, uint32_t childPayloadSize) {
			uint32_t payloadSize = sizeof(Signature) + sizeof(PacketHeader) + childPayloadSize;
			auto pPacket = test::CreateRandomPacket(payloadSize, PacketType::Secure_Signed);

			auto& signature = GetSecureSignedSignature(*pPacket);
			auto& childPacket = GetSecureSignedChildPacket(*pPacket);
			childPacket.Size = sizeof(PacketHeader) + childPayloadSize;
			childPacket.Type = PacketType::Push_Transactions;
			signature = SignPacket(keyPair, childPacket);
			return pPacket;
		}

		struct ReadCallbackParams {
			bool IsPacketValid;
			SocketOperationCode ReadCode;
			std::vector<uint8_t> ReadPacketBytes;
		};

		PacketIo::ReadCallback CreateReadCaptureCallback(ReadCallbackParams& capture) {
			return [&capture](auto code, const auto* pReadPacket) {
				capture.ReadCode = code;
				capture.IsPacketValid = !!pReadPacket;
				if (capture.IsPacketValid)
					capture.ReadPacketBytes = test::CopyPacketToBuffer(*pReadPacket);
			};
		}

		struct PacketIoReadTraits {
			static void Read(const TestContext& context, const PacketIo::ReadCallback& callback) {
				context.pSecureIo->read(callback);
			}
		};

		struct BatchPacketReaderReadTraits {
			static void Read(const TestContext& context, const PacketIo::ReadCallback& callback) {
				context.pSecureBatchReader->readMultiple(callback);
			}
		};
	}

#define READ_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PacketIoReadTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_BatchReader) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BatchPacketReaderReadTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	READ_TRAITS_BASED_TEST(ReadForwardsInnerReadError) {
		// Arrange:
		TestContext context;
		context.pMockPacketIo->queueRead(SocketOperationCode::Read_Error, nullptr);

		// Act:
		ReadCallbackParams capture;
		TTraits::Read(context, CreateReadCaptureCallback(capture));

		// Assert:
		EXPECT_EQ(SocketOperationCode::Read_Error, capture.ReadCode);
		EXPECT_FALSE(capture.IsPacketValid);
	}

	namespace {
		template<typename TReadTraits, typename TMutator>
		void RunFailedReadTest(SocketOperationCode expectedReadCode, uint32_t childPayloadSize, TMutator mutator) {
			// Arrange: create a (signed) packet
			TestContext context;
			auto pPacket = CreateSecureSignedPacket(context.RemoteKeyPair, childPayloadSize);
			auto& signature = GetSecureSignedSignature(*pPacket);
			auto& childPacket = GetSecureSignedChildPacket(*pPacket);

			// - mutate the packet or its data
			mutator(*pPacket, childPacket, signature);

			// - queue the read
			context.pMockPacketIo->queueRead(SocketOperationCode::Success, [pPacket](const auto*) { return pPacket; });

			// Act:
			ReadCallbackParams capture;
			TReadTraits::Read(context, CreateReadCaptureCallback(capture));

			// Assert:
			EXPECT_EQ(expectedReadCode, capture.ReadCode);
			EXPECT_FALSE(capture.IsPacketValid);
		}
	}

	READ_TRAITS_BASED_TEST(ReadFailsWhenEnvelopePacketTypeIsWrong) {
		// Assert: packet type must be Secure_Signed
		RunFailedReadTest<TTraits>(SocketOperationCode::Malformed_Data, 123, [](auto& packet, const auto&, const auto&) {
			packet.Type = PacketType::Push_Transactions;
		});
	}

	READ_TRAITS_BASED_TEST(ReadFailsWhenEnvelopePacketSizeIsTooSmall) {
		// Assert:
		RunFailedReadTest<TTraits>(SocketOperationCode::Malformed_Data, 0, [](auto& packet, auto& childPacket, const auto&) {
			--packet.Size;
			--childPacket.Size;
		});
	}

	READ_TRAITS_BASED_TEST(ReadFailsWhenEnvelopePacketSizeIsTooLargeRelativeToChildPacketSize) {
		// Assert:
		RunFailedReadTest<TTraits>(SocketOperationCode::Malformed_Data, 123, [](const auto&, auto& childPacket, const auto&) {
			--childPacket.Size;
		});
	}

	READ_TRAITS_BASED_TEST(ReadFailsWhenEnvelopePacketSizeIsTooSmallRelativeToChildPacketSize) {
		// Assert:
		RunFailedReadTest<TTraits>(SocketOperationCode::Malformed_Data, 123, [](const auto&, auto& childPacket, const auto&) {
			++childPacket.Size;
		});
	}

	READ_TRAITS_BASED_TEST(ReadFailsWhenEnvelopePacketSignatureDoesNotVerify) {
		// Assert:
		RunFailedReadTest<TTraits>(SocketOperationCode::Security_Error, 123, [](const auto&, const auto&, auto& signature) {
			signature[Signature_Size / 2] ^= 0xFF;
		});
	}

	namespace {
		template<typename TReadTraits, typename TAction>
		void RunReadSuccessPayloadTest(uint32_t childPayloadSize, TAction action) {
			// Arrange: create a (signed) packet
			TestContext context;
			auto pPacket = CreateSecureSignedPacket(context.RemoteKeyPair, childPayloadSize);
			auto& childPacket = GetSecureSignedChildPacket(*pPacket);

			// - queue the read
			context.pMockPacketIo->queueRead(SocketOperationCode::Success, [pPacket](const auto*) { return pPacket; });

			// Act:
			ReadCallbackParams capture;
			TReadTraits::Read(context, CreateReadCaptureCallback(capture));

			// Assert:
			ASSERT_EQ(SocketOperationCode::Success, capture.ReadCode);

			const auto& readPacket = reinterpret_cast<const Packet&>(capture.ReadPacketBytes[0]);
			ASSERT_EQ(sizeof(PacketHeader) + childPayloadSize, readPacket.Size);
			EXPECT_EQ(PacketType::Push_Transactions, readPacket.Type);

			EXPECT_EQ_MEMORY(childPacket.Data(), readPacket.Data(), childPayloadSize);
			action(readPacket);
		}
	}

	READ_TRAITS_BASED_TEST(ReadSucceedsWhenReadingEmptyPacketWithValidSignature) {
		// Assert:
		RunReadSuccessPayloadTest<TTraits>(0u, [](const auto& readPacket) {
			// Sanity:
			EXPECT_FALSE(!!readPacket.Data());
		});
	}

	READ_TRAITS_BASED_TEST(ReadSucceedsWhenReadingNonEmptyPacketWithValidSignature) {
		// Assert:
		RunReadSuccessPayloadTest<TTraits>(234u, [](const auto& readPacket) {
			// Sanity:
			EXPECT_TRUE(!!readPacket.Data());
		});
	}

	// endregion

	// region PacketIo - round trip

	TEST(TEST_CLASS, CanRoundtripWriteAndRead) {
		// Arrange:
		TestContext context;
		context.KeyPair = std::move(context.RemoteKeyPair); // the writer should emulate the remote so keys match for write and read

		// Act + Assert:
		test::AssertCanRoundtripPackets(*context.pMockPacketIo, *context.pSecureIo);
	}

	// endregion

	// region BatchPacketReader - readMultiple (multiple packets)

	TEST(TEST_CLASS, ReadSuccessWhenReadingMultiplePackets) {
		// Arrange: create two (signed) packets
		TestContext context;

		constexpr auto Data1_Size = 123u;
		auto pPacket1 = CreateSecureSignedPacket(context.RemoteKeyPair, Data1_Size);
		auto& childPacket1 = GetSecureSignedChildPacket(*pPacket1);

		constexpr auto Data2_Size = 222u;
		auto pPacket2 = CreateSecureSignedPacket(context.RemoteKeyPair, Data2_Size);
		auto& childPacket2 = GetSecureSignedChildPacket(*pPacket2);

		// - queue the read of both packets
		context.pMockPacketIo->queueRead(SocketOperationCode::Success, [pPacket1](const auto*) { return pPacket1; });
		context.pMockPacketIo->queueRead(SocketOperationCode::Success, [pPacket2](const auto*) { return pPacket2; });

		// Act:
		std::vector<ReadCallbackParams> captures;
		context.pSecureBatchReader->readMultiple([&captures](auto code, const auto* pReadPacket) {
			ReadCallbackParams capture;
			CreateReadCaptureCallback(capture)(code, pReadPacket);
			captures.push_back(capture);
		});

		// Assert: both packets were read
		ASSERT_EQ(2u, captures.size());
		ASSERT_EQ(SocketOperationCode::Success, captures[0].ReadCode);
		ASSERT_EQ(SocketOperationCode::Success, captures[1].ReadCode);

		const auto& readPacket1 = reinterpret_cast<const Packet&>(captures[0].ReadPacketBytes[0]);
		ASSERT_EQ(sizeof(PacketHeader) + Data1_Size, readPacket1.Size);
		EXPECT_EQ(PacketType::Push_Transactions, readPacket1.Type);
		EXPECT_EQ_MEMORY(childPacket1.Data(), readPacket1.Data(), Data1_Size);

		const auto& readPacket2 = reinterpret_cast<const Packet&>(captures[1].ReadPacketBytes[0]);
		ASSERT_EQ(sizeof(PacketHeader) + Data2_Size, readPacket2.Size);
		EXPECT_EQ(PacketType::Push_Transactions, readPacket2.Type);
		EXPECT_EQ_MEMORY(childPacket2.Data(), readPacket2.Data(), Data2_Size);
	}

	// endregion
}}
