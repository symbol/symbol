#include "catapult/ionet/WorkingBuffer.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

	namespace {
		constexpr size_t Default_Capacity = 4 * 1024;

		WorkingBuffer CreateWorkingBuffer() {
			PacketSocketOptions options;
			options.WorkingBufferSize = Default_Capacity;
			options.MaxPacketDataSize = 10 * 1024;
			return WorkingBuffer(options);
		}

		template<size_t N>
		std::array<uint8_t, N> AppendRandomData(WorkingBuffer& buffer) {
			auto data = test::GenerateRandomData<N>();
			auto context = buffer.prepareAppend();
			std::memcpy(boost::asio::buffer_cast<uint8_t*>(context.buffer()), data.data(), data.size());
			context.commit(N);
			return data;
		}

		template<typename TContainer>
		void AssertEqual(const TContainer& expected, const WorkingBuffer& actual) {
			EXPECT_TRUE(std::equal(expected.begin(), expected.end(), actual.data(), actual.data() + actual.size())); // raw data
			EXPECT_TRUE(std::equal(expected.begin(), expected.end(), actual.cbegin(), actual.cend())); // const iterator
		}
	}

	TEST(WorkingBufferTests, CanCreateWorkingBuffer) {
		// Act:
		PacketSocketOptions options;
		options.WorkingBufferSize = 2345;
		options.MaxPacketDataSize = 10 * 1024;
		auto buffer = WorkingBuffer(options);

		// Assert:
		EXPECT_EQ(0u, buffer.size());
		EXPECT_EQ(2345u, buffer.capacity());
	}

	TEST(WorkingBufferTests, CanAppendDataToWorkingBuffer) {
		// Arrange:
		auto buffer = CreateWorkingBuffer();

		// Act:
		auto data = AppendRandomData<100>(buffer);

		// Assert:
		EXPECT_EQ(100u, buffer.size());
		EXPECT_EQ(Default_Capacity, buffer.capacity());
		AssertEqual(data, buffer);
	}

	TEST(WorkingBufferTests, CanAppendDataToWorkingBufferMultipleTimes) {
		// Arrange:
		auto buffer = CreateWorkingBuffer();

		// Act:
		std::vector<uint8_t> allData;
		for (auto i = 0u; i < 5; ++i) {
			auto data = AppendRandomData<100>(buffer);
			allData.insert(allData.end(), data.cbegin(), data.cend());
		}

		// Assert:
		EXPECT_EQ(500u, buffer.size());
		EXPECT_LE(Default_Capacity, buffer.capacity());
		AssertEqual(allData, buffer);
	}

	TEST(WorkingBufferTests, CanAbandonUncommittedAppendData) {
		// Arrange:
		auto buffer = CreateWorkingBuffer();

		// Act: don't call context.commit
		{
			auto data = test::GenerateRandomData<100>();
			auto context = buffer.prepareAppend();
			std::memcpy(boost::asio::buffer_cast<uint8_t*>(context.buffer()), &data, data.size());
		}

		// Assert:
		EXPECT_EQ(0u, buffer.size());
		EXPECT_EQ(Default_Capacity, buffer.capacity());
	}

	namespace {
		void SetPacketSize(WorkingBuffer& buffer, uint32_t size) {
			*reinterpret_cast<uint32_t*>(const_cast<uint8_t*>(buffer.data())) = size;
		}
	}

	TEST(WorkingBufferTests, CanExtractPacketFromWorkingBuffer) {
		// Arrange:
		auto buffer = CreateWorkingBuffer();
		AppendRandomData<100>(buffer);
		SetPacketSize(buffer, 25);

		// Act:
		auto extractor = buffer.preparePacketExtractor();
		const Packet* pPacket;
		auto result = extractor.tryExtractNextPacket(pPacket);

		// Assert:
		auto pPacketBuffer = reinterpret_cast<const uint8_t*>(pPacket);
		EXPECT_EQ(PacketExtractResult::Success, result);
		EXPECT_EQ(100u, buffer.size());
		ASSERT_TRUE(!!pPacket);
		EXPECT_TRUE(std::equal(buffer.cbegin(), buffer.cbegin() + 25, pPacketBuffer, pPacketBuffer + pPacket->Size));
	}

	TEST(WorkingBufferTests, CannotExtractPacketFromWorkingBufferWithDataSizeExceedingMaxPacketDataSize) {
		// Arrange:
		constexpr auto Packet_Buffer_Size = 25u;
		PacketSocketOptions options;
		options.WorkingBufferSize = 2345;
		options.MaxPacketDataSize = Packet_Buffer_Size - sizeof(PacketHeader) - 1;

		auto buffer = WorkingBuffer(options);
		AppendRandomData<100>(buffer);
		SetPacketSize(buffer, Packet_Buffer_Size);

		// Act:
		auto extractor = buffer.preparePacketExtractor();
		const Packet* pPacket;
		auto result = extractor.tryExtractNextPacket(pPacket);

		// Assert:
		EXPECT_EQ(PacketExtractResult::Packet_Error, result);
		EXPECT_EQ(100u, buffer.size());
		EXPECT_FALSE(!!pPacket);
	}

	TEST(WorkingBufferTests, CanConsumePacketFromWorkingBuffer) {
		// Arrange:
		auto buffer = CreateWorkingBuffer();
		AppendRandomData<100>(buffer);
		SetPacketSize(buffer, 25);

		// Act:
		auto extractor = buffer.preparePacketExtractor();
		const Packet* pPacket;
		extractor.tryExtractNextPacket(pPacket);
		extractor.consume();

		// Assert:
		EXPECT_EQ(75u, buffer.size());
	}
}}
