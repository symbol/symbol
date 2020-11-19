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

#include "catapult/ionet/ReadRateMonitorPacketIo.h"
#include "catapult/ionet/PacketPayloadFactory.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/PacketIoTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS ReadRateMonitorPacketIoTests

	namespace {
		struct TestContext {
		public:
			TestContext()
					: pMockPacketIo(std::make_shared<mocks::MockPacketIo>())
					, pReadRateMonitorIo(CreateReadRateMonitorPacketIo(pMockPacketIo, [&sizes = ReadPacketSizes](auto size) {
						sizes.push_back(size);
					}))
					, pReadRateMonitorReader(CreateReadRateMonitorBatchPacketReader(pMockPacketIo, [&sizes = ReadPacketSizes](auto size) {
						sizes.push_back(size);
					}))
			{}

		public:
			std::shared_ptr<mocks::MockPacketIo> pMockPacketIo;
			std::shared_ptr<PacketIo> pReadRateMonitorIo;
			std::shared_ptr<BatchPacketReader> pReadRateMonitorReader;
			std::vector<uint32_t> ReadPacketSizes;
		};
	}

	// region PacketIo - write

	TEST(TEST_CLASS, WriteDoesNotCallReadSizeConsumer) {
		// Arrange:
		TestContext context;
		context.pMockPacketIo->queueWrite(SocketOperationCode::Success);

		auto entities = std::vector<std::shared_ptr<model::VerifiableEntity>>{ test::CreateRandomEntityWithSize<>(126) };
		auto payload = PacketPayloadFactory::FromEntities(PacketType::Push_Transactions, entities);

		// Act:
		SocketOperationCode writeCode;
		context.pReadRateMonitorIo->write(payload, [&writeCode](auto code) {
			writeCode = code;
		});

		const auto& writtenPacket = context.pMockPacketIo->writtenPacketAt<Packet>(0);

		// Assert:
		EXPECT_EQ(SocketOperationCode::Success, writeCode);

		ASSERT_EQ(sizeof(PacketHeader) + 126, writtenPacket.Size);
		EXPECT_EQ(PacketType::Push_Transactions, writtenPacket.Type);
		EXPECT_EQ_MEMORY(entities[0].get(), writtenPacket.Data(), entities[0]->Size);

		// - callback was not called
		EXPECT_TRUE(context.ReadPacketSizes.empty());
	}

	// endregion

	// region PacketIo - read

	namespace {
		struct PacketIoReadTraits {
			static void Read(const TestContext& context, const PacketIo::ReadCallback& callback) {
				context.pReadRateMonitorIo->read(callback);
			}
		};

		struct BatchPacketReaderReadTraits {
			static void Read(const TestContext& context, const PacketIo::ReadCallback& callback) {
				context.pReadRateMonitorReader->readMultiple(callback);
			}
		};
	}

#define READ_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PacketIoReadTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_BatchReader) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BatchPacketReaderReadTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	READ_TRAITS_BASED_TEST(ReadErrorDoesNotCallReadSizeConsumer) {
		// Arrange:
		TestContext context;
		context.pMockPacketIo->queueRead(SocketOperationCode::Read_Error, nullptr);

		// Act:
		test::PacketIoReadCallbackParams capture;
		TTraits::Read(context, test::CreateReadCaptureCallback(capture));

		// Assert:
		EXPECT_EQ(SocketOperationCode::Read_Error, capture.ReadCode);
		EXPECT_FALSE(capture.IsPacketValid);

		// - callback was not called
		EXPECT_TRUE(context.ReadPacketSizes.empty());
	}

	READ_TRAITS_BASED_TEST(ReadSuccessCallsReadSizeConsumer) {
		// Arrange: create a packet
		TestContext context;
		constexpr auto Data_Size = 123u;
		auto pPacket = test::CreateRandomPacket(Data_Size, PacketType::Push_Transactions);

		// - queue the read
		context.pMockPacketIo->queueRead(SocketOperationCode::Success, [pPacket](const auto*) { return pPacket; });

		// Act:
		test::PacketIoReadCallbackParams capture;
		TTraits::Read(context, test::CreateReadCaptureCallback(capture));

		// Assert:
		ASSERT_EQ(SocketOperationCode::Success, capture.ReadCode);

		const auto& readPacket = reinterpret_cast<const Packet&>(capture.ReadPacketBytes[0]);
		ASSERT_EQ(sizeof(PacketHeader) + Data_Size, readPacket.Size);
		EXPECT_EQ(PacketType::Push_Transactions, readPacket.Type);

		EXPECT_EQ_MEMORY(pPacket->Data(), readPacket.Data(), Data_Size);

		// - callback was called
		EXPECT_EQ(std::vector<uint32_t>{ sizeof(PacketHeader) + Data_Size }, context.ReadPacketSizes);
	}

	// endregion

	// region BatchPacketReader - readMultiple (multiple packets)

	TEST(TEST_CLASS, ReadSuccessWhenReadingMultiplePackets) {
		// Arrange: create two packets
		TestContext context;

		constexpr auto Data1_Size = 123u;
		auto pPacket1 = test::CreateRandomPacket(Data1_Size, PacketType::Push_Transactions);

		constexpr auto Data2_Size = 222u;
		auto pPacket2 = test::CreateRandomPacket(Data2_Size, PacketType::Push_Block);

		// - queue the read of both packets
		context.pMockPacketIo->queueRead(SocketOperationCode::Success, [pPacket1](const auto*) { return pPacket1; });
		context.pMockPacketIo->queueRead(SocketOperationCode::Success, [pPacket2](const auto*) { return pPacket2; });

		// Act:
		std::vector<test::PacketIoReadCallbackParams> captures;
		context.pReadRateMonitorReader->readMultiple(test::CreateReadCaptureCallback(captures));

		// Assert: both packets were read
		ASSERT_EQ(2u, captures.size());
		ASSERT_EQ(SocketOperationCode::Success, captures[0].ReadCode);
		ASSERT_EQ(SocketOperationCode::Success, captures[1].ReadCode);

		const auto& readPacket1 = reinterpret_cast<const Packet&>(captures[0].ReadPacketBytes[0]);
		ASSERT_EQ(sizeof(PacketHeader) + Data1_Size, readPacket1.Size);
		EXPECT_EQ(PacketType::Push_Transactions, readPacket1.Type);
		EXPECT_EQ_MEMORY(pPacket1->Data(), readPacket1.Data(), Data1_Size);

		const auto& readPacket2 = reinterpret_cast<const Packet&>(captures[1].ReadPacketBytes[0]);
		ASSERT_EQ(sizeof(PacketHeader) + Data2_Size, readPacket2.Size);
		EXPECT_EQ(PacketType::Push_Block, readPacket2.Type);
		EXPECT_EQ_MEMORY(pPacket2->Data(), readPacket2.Data(), Data2_Size);

		// - callback was called for each packet
		EXPECT_EQ(
				std::vector<uint32_t>({ sizeof(PacketHeader) + Data1_Size, sizeof(PacketHeader) + Data2_Size }),
				context.ReadPacketSizes);
	}

	// endregion
}}
