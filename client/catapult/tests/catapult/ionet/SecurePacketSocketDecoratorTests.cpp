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

#include "catapult/ionet/SecurePacketSocketDecorator.h"
#include "catapult/ionet/PacketPayloadFactory.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/utils/FileSize.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/PacketIoTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS SecurePacketSocketDecoratorTests

	namespace {
		// region MockPacketSocket

		class MockPacketSocket : public PacketSocket, public mocks::MockPacketIo {
		public:
			MockPacketSocket()
					: m_numStatsCalls(0)
					, m_numCloseCalls(0)
					, m_numBufferedCalls(0)
					, m_pBufferedIo(std::make_shared<mocks::MockPacketIo>()) {
				// allow a write / read roundtrip so the buffered roundtrip test will pass
				m_pBufferedIo->queueWrite(SocketOperationCode::Success);
				m_pBufferedIo->queueRead(SocketOperationCode::Success, [](const auto* pWrittenPacket) {
					auto pWrittenPacketCopy = utils::MakeSharedWithSize<Packet>(pWrittenPacket->Size);
					std::memcpy(pWrittenPacketCopy.get(), pWrittenPacket, pWrittenPacket->Size);
					return pWrittenPacketCopy;
				});
			}

		public:
			size_t numStatsCalls() const {
				return m_numStatsCalls;
			}

			size_t numCloseCalls() const {
				return m_numCloseCalls;
			}

			size_t numBufferedCalls() const {
				return m_numBufferedCalls;
			}

			auto mockBufferedIo() {
				return m_pBufferedIo;
			}

		public:
			void read(const ReadCallback& callback) override {
				mocks::MockPacketIo::read(callback);
			}

			void write(const PacketPayload& payload, const WriteCallback& callback) override {
				mocks::MockPacketIo::write(payload, callback);
			}

			void readMultiple(const ReadCallback& callback) override {
				mocks::MockPacketIo::readMultiple(callback);
			}

		public:
			void stats(const StatsCallback& callback) override {
				callback({ true, ++m_numStatsCalls });
			}

			void close() override{
				++m_numCloseCalls;
			}

			std::shared_ptr<PacketIo> buffered() override {
				++m_numBufferedCalls;
				return m_pBufferedIo;
			}

		private:
			size_t m_numStatsCalls;
			size_t m_numCloseCalls;
			size_t m_numBufferedCalls;

			std::shared_ptr<mocks::MockPacketIo> m_pBufferedIo;
		};

		// endregion

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

		struct TestContext {
		public:
			explicit TestContext(ConnectionSecurityMode securityMode, uint32_t maxPacketDataSize = std::numeric_limits<uint32_t>::max())
					: TestContext(securityMode, utils::FileSize::FromBytes(maxPacketDataSize))
			{}

		private:
			TestContext(ConnectionSecurityMode securityMode, const utils::FileSize& maxPacketDataSize)
					: pMockPacketSocket(std::make_shared<MockPacketSocket>())
					, KeyPair(test::GenerateKeyPair())
					, RemoteKey(KeyPair.publicKey()) // use same public key so secure packets can be signed and verified
					, pSecureSocket(Secure(pMockPacketSocket, securityMode, KeyPair, RemoteKey, maxPacketDataSize))
			{}

		public:
			IoView normalIoView() {
				return { pSecureSocket, pMockPacketSocket };
			}

			IoView bufferedIoView() {
				return { pSecureSocket->buffered(), pMockPacketSocket->mockBufferedIo() };
			}

		public:
			std::shared_ptr<MockPacketSocket> pMockPacketSocket;
			crypto::KeyPair KeyPair;
			Key RemoteKey;
			std::shared_ptr<PacketSocket> pSecureSocket;
		};

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

#define SOCKET_TRAITS_BASED_TEST(TEST_NAME) \
	template<ConnectionSecurityMode SecurityMode> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, SecurityModeNone##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ConnectionSecurityMode::None>(); } \
	TEST(TEST_CLASS, SecurityModeSigned##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ConnectionSecurityMode::Signed>(); } \
	template<ConnectionSecurityMode SecurityMode> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region ConnectionSecurityMode - common

	SOCKET_TRAITS_BASED_TEST(CanRoundtripPackets) {
		// Arrange:
		TestContext context(SecurityMode);

		// Act + Assert:
		test::AssertCanRoundtripPackets(*context.pMockPacketSocket, *context.pSecureSocket);
	}

	SOCKET_TRAITS_BASED_TEST(CanRoundtripPacketsWithReadMultiple) {
		// Arrange:
		TestContext context(SecurityMode);

		// Act + Assert:
		test::AssertCanRoundtripPackets(*context.pMockPacketSocket, *context.pSecureSocket, *context.pSecureSocket);
	}

	SOCKET_TRAITS_BASED_TEST(CanRoundtripBufferedPackets) {
		// Arrange:
		TestContext context(SecurityMode);

		// Act + Assert:
		test::AssertCanRoundtripPackets(*context.pMockPacketSocket, *context.pSecureSocket->buffered());

		// Sanity:
		EXPECT_EQ(1u, context.pMockPacketSocket->numBufferedCalls());
	}

	SOCKET_TRAITS_BASED_TEST(CanAccessStats) {
		// Arrange:
		TestContext context(SecurityMode);

		// Act:
		PacketSocket::Stats capturedStats;
		context.pSecureSocket->stats([&capturedStats](const auto& stats) {
			capturedStats = stats;
		});

		// Assert: NumUnprocessedBytes is set to call count by MockPacketSocket
		EXPECT_EQ(1u, context.pMockPacketSocket->numStatsCalls());
		EXPECT_EQ(1u, capturedStats.NumUnprocessedBytes);
	}

	SOCKET_TRAITS_BASED_TEST(CanClose) {
		// Arrange:
		TestContext context(SecurityMode);

		// Act:
		context.pSecureSocket->close();

		// Assert:
		EXPECT_EQ(1u, context.pMockPacketSocket->numCloseCalls());
	}

	// endregion

	// region ConnectionSecurityMode - None

	TEST(TEST_CLASS, SecurityModeNone_DoesNotDecorateSocket) {
		// Arrange:
		TestContext context(ConnectionSecurityMode::None);

		// Act + Assert:
		EXPECT_EQ(context.pMockPacketSocket, context.pSecureSocket);
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

	TEST(TEST_CLASS, SecurityModeSigned_DecoratesSocket) {
		// Arrange:
		TestContext context(ConnectionSecurityMode::Signed);

		// Act + Assert
		EXPECT_NE(context.pMockPacketSocket, context.pSecureSocket);
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
