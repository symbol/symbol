#include "catapult/ionet/WorkingBuffer.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS WorkingBufferTests

	namespace {
		constexpr size_t Default_Capacity = 4 * 1024;

		WorkingBuffer CreateWorkingBuffer(size_t sensitivity = 10) {
			PacketSocketOptions options;
			options.WorkingBufferSize = Default_Capacity;
			options.WorkingBufferSensitivity = sensitivity;
			options.MaxPacketDataSize = 15 * 1024;
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

		void SetPacketSize(WorkingBuffer& buffer, uint32_t size) {
			*reinterpret_cast<uint32_t*>(const_cast<uint8_t*>(buffer.data())) = size;
		}

		template<typename TContainer>
		void AssertEqual(const TContainer& expected, const WorkingBuffer& actual) {
			EXPECT_TRUE(std::equal(expected.begin(), expected.end(), actual.data(), actual.data() + actual.size())); // raw data
			EXPECT_TRUE(std::equal(expected.begin(), expected.end(), actual.begin(), actual.end())); // const iterator
		}
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateWorkingBuffer) {
		// Arrange:
		PacketSocketOptions options;
		options.WorkingBufferSize = 2345;
		options.WorkingBufferSensitivity = 10;
		options.MaxPacketDataSize = 10 * 1024;

		// Act:
		auto buffer = WorkingBuffer(options);

		// Assert:
		EXPECT_EQ(0u, buffer.size());
		EXPECT_EQ(2345u, buffer.capacity());
	}

	// endregion

	// region prepareAppend

	TEST(TEST_CLASS, CanAppendDataToWorkingBuffer) {
		// Arrange:
		auto buffer = CreateWorkingBuffer();

		// Act:
		auto data = AppendRandomData<100>(buffer);

		// Assert:
		EXPECT_EQ(100u, buffer.size());
		EXPECT_EQ(Default_Capacity, buffer.capacity());
		AssertEqual(data, buffer);
	}

	TEST(TEST_CLASS, CanAppendDataToWorkingBufferMultipleTimes) {
		// Arrange:
		auto buffer = CreateWorkingBuffer();

		// Act:
		std::vector<uint8_t> allData;
		for (auto i = 0u; i < 5u; ++i) {
			auto data = AppendRandomData<100>(buffer);
			allData.insert(allData.end(), data.cbegin(), data.cend());
		}

		// Assert:
		EXPECT_EQ(500u, buffer.size());
		EXPECT_LE(Default_Capacity, buffer.capacity());
		AssertEqual(allData, buffer);
	}

	TEST(TEST_CLASS, CanAbandonUncommittedAppendData) {
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

	// endregion

	// region preparePacketExtractor

	TEST(TEST_CLASS, CanExtractPacketFromWorkingBuffer) {
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
		EXPECT_TRUE(std::equal(buffer.begin(), buffer.begin() + 25, pPacketBuffer, pPacketBuffer + pPacket->Size));
	}

	TEST(TEST_CLASS, CannotExtractPacketFromWorkingBufferWithDataSizeExceedingMaxPacketDataSize) {
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

	TEST(TEST_CLASS, CanConsumePacketFromWorkingBuffer) {
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

	// endregion

	// region memory management

	namespace {
		void AppendAndConsumeRandomData(WorkingBuffer& buffer, uint32_t multiple) {
			// add a large packet to the buffer (notice that chunks cannot be larger than Default_Capacity because
			// prepareAppend prepares a buffer of size Default_Capacity)
			for (auto i = 0u; i < multiple; ++i)
				AppendRandomData<Default_Capacity>(buffer);

			SetPacketSize(buffer, multiple * Default_Capacity);

			// consume the data
			auto extractor = buffer.preparePacketExtractor();
			const Packet* pPacket;
			extractor.tryExtractNextPacket(pPacket);
			extractor.consume();
		}
	}

	TEST(TEST_CLASS, BufferExpandsToFitIncreasingDataSizes) {
		// Arrange: create a working buffer with sensitivity 5
		auto buffer = CreateWorkingBuffer(5);

		// Act: keep appending to the working buffer without committing
		std::vector<uint8_t> allData;
		for (auto i = 0u; i < 10u; ++i) {
			auto data = AppendRandomData<Default_Capacity / 4>(buffer);
			allData.insert(allData.end(), data.cbegin(), data.cend());
		}

		// Assert:
		EXPECT_EQ(Default_Capacity / 4 * 10, buffer.size());
		EXPECT_LE(Default_Capacity / 4 * 10, buffer.capacity());
		AssertEqual(allData, buffer);
	}

	TEST(TEST_CLASS, BufferIsShrunkToReclaimMemoryInPresenceOfSmallerDataSizesWhenMemoryReclamationIsEnabled) {
		// Arrange: create a working buffer with sensitivity 5
		auto buffer = CreateWorkingBuffer(5);

		// - append and consume a large packet (append is done in three chunks)
		AppendAndConsumeRandomData(buffer, 3);
		auto isShrinkWrapped = Default_Capacity * 3 == buffer.capacity();

		// Sanity:
		EXPECT_EQ(0u, buffer.size());
		EXPECT_LE(Default_Capacity * 3, buffer.capacity());

		// Act: append small data
		std::vector<uint8_t> allData;
		std::vector<size_t> capacities;
		std::vector<size_t> capacityChangeIndexes;
		for (auto i = 0u; i < 12u; ++i) {
			auto data = AppendRandomData<10>(buffer);
			allData.insert(allData.end(), data.cbegin(), data.cend());

			if (capacities.empty() || buffer.capacity() != capacities.back()) {
				capacities.push_back(buffer.capacity());
				capacityChangeIndexes.push_back(i);
			}
		}

		// Assert: capacity should be reduced
		EXPECT_EQ(120u, buffer.size());
		EXPECT_GE(Default_Capacity * 3, buffer.capacity());
		AssertEqual(allData, buffer);

		// - capacity is only reduced at sensitivity intervals (15 samples should result in 3 reclamation attempts)
		// -  0 => initial capacity
		// -  1 => first attempt (2 + 3)  ; shrink-wraps large allocation (only if initial large allocation is not exact)
		// -  6 => second attempt (7 + 3) ; reclaims memory based on history of small samples
		// - 11 => third attempt (12 + 3) ; no reclamation because samples are same as in (2)
		std::vector<size_t> expectedCapacityChangeIndexes{ 0, 1, 6 };
		if (isShrinkWrapped)
			expectedCapacityChangeIndexes.erase(expectedCapacityChangeIndexes.cbegin() + 1);

		EXPECT_EQ(expectedCapacityChangeIndexes, capacityChangeIndexes);
		EXPECT_EQ(expectedCapacityChangeIndexes.size(), capacities.size());
		for (auto i = 0u; i < capacities.size() - 1; ++i)
			EXPECT_LE(capacities[i + 1], capacities[i]) << "comparing capacities at " << i + 1 << " and " << i;
	}

	TEST(TEST_CLASS, BufferIsNotShrunkToReclaimMemoryInPresenceOfSmallerDataSizesWhenMemoryReclamationIsDisabled) {
		// Arrange: create a working buffer with memory reclamation disabled
		auto buffer = CreateWorkingBuffer(0);

		// - append and consume a large packet
		AppendAndConsumeRandomData(buffer, 3);

		// Sanity:
		EXPECT_EQ(0u, buffer.size());
		EXPECT_LE(Default_Capacity * 3, buffer.capacity());

		// Act: append small data
		std::vector<uint8_t> allData;
		for (auto i = 0u; i < 12u; ++i) {
			auto data = AppendRandomData<10>(buffer);
			allData.insert(allData.end(), data.cbegin(), data.cend());
		}

		// Assert: capacity should not be reduced
		EXPECT_EQ(120u, buffer.size());
		EXPECT_LE(Default_Capacity * 3, buffer.capacity());
		AssertEqual(allData, buffer);
	}

	// endregion
}}
