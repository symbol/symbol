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

#include "catapult/ionet/SecureSignedPacketSocketDecorator.h"
#include "catapult/utils/FileSize.h"
#include "tests/test/core/PacketSocketDecoratorTests.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS SecureSignedPacketSocketDecoratorTests

	namespace {
		// region IoView

		struct IoView {
		public:
			IoView(const std::shared_ptr<PacketIo>& pIo, const std::shared_ptr<mocks::MockPacketIo>& pMockIo)
					: m_pIo(pIo)
					, m_pMockIo(pMockIo)
					, Io(*m_pIo)
					, MockIo(*m_pMockIo)
			{}

		private:
			std::shared_ptr<PacketIo> m_pIo;
			std::shared_ptr<mocks::MockPacketIo> m_pMockIo;

		public:
			PacketIo& Io;
			mocks::MockPacketIo& MockIo;
		};

		// endregion

		// region TestContext

		struct TestContext {
		public:
			explicit TestContext(ConnectionSecurityMode securityMode, uint32_t maxPacketDataSize = std::numeric_limits<uint32_t>::max())
					: TestContext(securityMode, utils::FileSize::FromBytes(maxPacketDataSize))
			{}

		private:
			TestContext(ConnectionSecurityMode securityMode, utils::FileSize maxPacketDataSize)
					: pMockPacketSocket(std::make_shared<mocks::MockPacketSocket>())
					, KeyPair(test::GenerateKeyPair())
					, RemoteKey(KeyPair.publicKey()) // use same public key so secure packets can be signed and verified
					, pDecoratedSocket(AddSecureSigned(pMockPacketSocket, securityMode, KeyPair, RemoteKey, maxPacketDataSize))
			{}

		public:
			IoView normalIoView() {
				return { pDecoratedSocket, pMockPacketSocket };
			}

			IoView bufferedIoView() {
				return { pDecoratedSocket->buffered(), pMockPacketSocket->mockBufferedIo() };
			}

		public:
			std::shared_ptr<mocks::MockPacketSocket> pMockPacketSocket;
			crypto::KeyPair KeyPair;
			Key RemoteKey;
			std::shared_ptr<PacketSocket> pDecoratedSocket;
		};

		// endregion
	}

	// region ConnectionSecurityMode - all

	namespace {
		struct NoneTraits {
			struct TestContextType : public TestContext {
				TestContextType() : TestContext(ConnectionSecurityMode::None)
				{}
			};
		};

		struct SignedTraits {
			struct TestContextType : public TestContext {
				TestContextType() : TestContext(ConnectionSecurityMode::Signed)
				{}
			};
		};
	}

	DEFINE_PACKET_SOCKET_DECORATOR_TESTS(NoneTraits, SecurityModeNone_)
	DEFINE_PACKET_SOCKET_DECORATOR_TESTS(SignedTraits, SecurityModeSigned_)

	// endregion

	// region ConnectionSecurityMode - None

	namespace {
		void AssertNormalPacketWriteCode(const IoView& view, PacketType expectedPacketType, PacketType packetType) {
			// Arrange:
			auto payload = PacketPayload(test::CreateRandomPacket(100, packetType));

			// - allow the mock io to accept a write
			view.MockIo.queueWrite(SocketOperationCode::Success);

			// Act:
			SocketOperationCode writeCode;
			view.Io.write(payload, [&writeCode](auto code) {
				writeCode = code;
			});

			// Assert:
			EXPECT_EQ(SocketOperationCode::Success, writeCode);

			const auto& writtenPacket = view.MockIo.writtenPacketAt<Packet>(0);
			EXPECT_EQ(expectedPacketType, writtenPacket.Type);
		}
	}

	TEST(TEST_CLASS, SecurityModeNone_DoesNotDecorateSocket) {
		// Arrange:
		TestContext context(ConnectionSecurityMode::None);

		// Act + Assert:
		EXPECT_EQ(context.pMockPacketSocket, context.pDecoratedSocket);
	}

	TEST(TEST_CLASS, SecurityModeNone_DoesNotWriteSecurePackets) {
		// Arrange:
		TestContext context(ConnectionSecurityMode::None);

		// Act + Assert:
		AssertNormalPacketWriteCode(context.normalIoView(), PacketType::Pull_Transactions, PacketType::Pull_Transactions);
	}

	TEST(TEST_CLASS, SecurityModeNone_DoesNotWriteSecureBufferedPackets) {
		// Arrange:
		TestContext context(ConnectionSecurityMode::None);

		// Act + Assert:
		AssertNormalPacketWriteCode(context.bufferedIoView(), PacketType::Pull_Transactions, PacketType::Pull_Transactions);
	}

	// endregion

	// region ConnectionSecurityMode - Signed

	namespace {
		void AssertMalformedDataWrite(const IoView& view, const PacketPayload& payload) {
			// Arrange:
			view.MockIo.queueWrite(SocketOperationCode::Success);

			// Act:
			SocketOperationCode writeCode;
			view.Io.write(payload, [&writeCode](auto code) {
				writeCode = code;
			});

			// Assert:
			EXPECT_EQ(SocketOperationCode::Malformed_Data, writeCode);
		}
	}

	TEST(TEST_CLASS, SecurityModeSigned_DecoratesSocket) {
		// Arrange:
		TestContext context(ConnectionSecurityMode::Signed);

		// Act + Assert
		EXPECT_NE(context.pMockPacketSocket, context.pDecoratedSocket);
	}

	TEST(TEST_CLASS, SecurityModeSigned_WritesSecurePackets) {
		// Arrange:
		TestContext context(ConnectionSecurityMode::Signed);

		// Act + Assert:
		AssertNormalPacketWriteCode(context.normalIoView(), PacketType::Secure_Signed, PacketType::Pull_Transactions);
	}

	TEST(TEST_CLASS, SecurityModeSigned_WritesSecureBufferedPackets) {
		// Arrange:
		TestContext context(ConnectionSecurityMode::Signed);

		// Act + Assert:
		AssertNormalPacketWriteCode(context.bufferedIoView(), PacketType::Secure_Signed, PacketType::Pull_Transactions);
	}

	TEST(TEST_CLASS, SecurityModeSigned_EnforcesMaxPacketDataSizeOnWrite) {
		// Arrange:
		TestContext context(ConnectionSecurityMode::Signed, 99);

		auto payload = PacketPayload(test::CreateRandomPacket(100, PacketType::Pull_Transactions));

		// Act + Assert:
		AssertMalformedDataWrite(context.normalIoView(), payload);
	}

	TEST(TEST_CLASS, SecurityModeSigned_EnforcesMaxPacketDataSizeOnBufferedWrite) {
		// Arrange:
		TestContext context(ConnectionSecurityMode::Signed, 99);

		auto payload = PacketPayload(test::CreateRandomPacket(100, PacketType::Pull_Transactions));

		// Act + Assert:
		AssertMalformedDataWrite(context.bufferedIoView(), payload);
	}

	// endregion
}}
