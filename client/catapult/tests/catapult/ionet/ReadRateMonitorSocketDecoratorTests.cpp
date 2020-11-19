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

#include "catapult/ionet/ReadRateMonitorSocketDecorator.h"
#include "catapult/ionet/RateMonitor.h"
#include "catapult/utils/FileSize.h"
#include "tests/test/core/PacketSocketDecoratorTests.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/nodeps/TimeSupplier.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS ReadRateMonitorSocketDecoratorTests

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
			explicit TestContext(uint16_t numBuckets)
					: NumRateExceededTriggers(0)
					, pMockPacketSocket(std::make_shared<mocks::MockPacketSocket>())
					, pDecoratedSocket(AddReadRateMonitor(
							pMockPacketSocket,
							{ numBuckets, utils::TimeSpan::FromMilliseconds(111), utils::FileSize::FromBytes(1000) },
							test::CreateTimeSupplierFromMilliseconds({ 1 }),
							[&numRateExceededTriggers = NumRateExceededTriggers]() { ++numRateExceededTriggers; }))
			{}

		public:
			IoView normalIoView() {
				return { pDecoratedSocket, pMockPacketSocket };
			}

			IoView bufferedIoView() {
				return { pDecoratedSocket->buffered(), pMockPacketSocket->mockBufferedIo() };
			}

		public:
			size_t NumRateExceededTriggers;
			std::shared_ptr<mocks::MockPacketSocket> pMockPacketSocket;
			std::shared_ptr<PacketSocket> pDecoratedSocket;
		};

		// endregion
	}

	// region all

	namespace {
		struct DisabledTraits {
			struct TestContextType : public TestContext {
				TestContextType() : TestContext(0)
				{}
			};
		};

		struct EnabledTraits {
			struct TestContextType : public TestContext {
				TestContextType() : TestContext(1)
				{}
			};
		};
	}

	DEFINE_PACKET_SOCKET_DECORATOR_TESTS(DisabledTraits, RateMonitorDisabled_)
	DEFINE_PACKET_SOCKET_DECORATOR_TESTS(EnabledTraits, RateMonitorEnabled_)

	// endregion

	// region disabled

	namespace {
		void ReadPacket(const IoView& view, uint32_t size) {
			auto pPacket = test::CreateRandomPacket(size, PacketType::Push_Block);
			view.MockIo.queueRead(SocketOperationCode::Success, [pPacket](const auto*) { return pPacket; });
			view.Io.read([](auto, const auto*) {});
		}

		template<typename TTestContext>
		void ReadMultiplePacket(const TTestContext& context, uint32_t size) {
			auto pPacket = test::CreateRandomPacket(size, PacketType::Push_Block);
			context.pMockPacketSocket->queueRead(SocketOperationCode::Success, [pPacket](const auto*) { return pPacket; });
			context.pDecoratedSocket->readMultiple([](auto, const auto*) {});
		}
	}

	TEST(TEST_CLASS, RateMonitorDisabled_DoesNotDecorateSocket) {
		// Arrange:
		DisabledTraits::TestContextType context;

		// Act + Assert:
		EXPECT_EQ(context.pMockPacketSocket, context.pDecoratedSocket);
	}

	TEST(TEST_CLASS, RateMonitorDisabled_DoesNotEnforceRateLimit) {
		// Arrange:
		DisabledTraits::TestContextType context;

		// Act: (400 + sizeof(PacketHeader)) * 3 > 1000
		ReadPacket(context.normalIoView(), 400);
		ReadPacket(context.bufferedIoView(), 400);
		ReadMultiplePacket(context, 400);

		// Assert:
		EXPECT_EQ(0u, context.NumRateExceededTriggers);
	}

	// endregion

	// region enabled

	TEST(TEST_CLASS, RateMonitorEnabled_DecoratesSocket) {
		// Arrange:
		EnabledTraits::TestContextType context;

		// Act + Assert
		EXPECT_NE(context.pMockPacketSocket, context.pDecoratedSocket);
	}

	TEST(TEST_CLASS, RateMonitorEnabled_EnforcesRateLimit) {
		// Arrange:
		EnabledTraits::TestContextType context;

		// Act: (400 + sizeof(PacketHeader)) * 3 > 1000
		ReadPacket(context.normalIoView(), 400);
		ReadPacket(context.bufferedIoView(), 400);
		ReadMultiplePacket(context, 400);

		// Assert:
		EXPECT_EQ(1u, context.NumRateExceededTriggers);
	}

	namespace {
		void AssertSingleByteTriggersRateLimit(const IoView& view, size_t& numRateExceededTriggers) {
			// Act:
			ReadPacket(view, 1000 - sizeof(PacketHeader));

			// Sanity:
			EXPECT_EQ(0u, numRateExceededTriggers);

			// Act:
			ReadPacket(view, 1);

			// Assert:
			EXPECT_EQ(1u, numRateExceededTriggers);
		}
	}

	TEST(TEST_CLASS, RateMonitorEnabled_SingleByteTriggersRateLimit) {
		// Arrange:
		EnabledTraits::TestContextType context;

		// Act + Assert:
		AssertSingleByteTriggersRateLimit(context.normalIoView(), context.NumRateExceededTriggers);
	}

	TEST(TEST_CLASS, RateMonitorEnabled_SingleBufferedByteTriggersRateLimit) {
		// Arrange:
		EnabledTraits::TestContextType context;

		// Act + Assert:
		AssertSingleByteTriggersRateLimit(context.bufferedIoView(), context.NumRateExceededTriggers);
	}

	// endregion
}}
