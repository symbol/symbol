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

#include "catapult/ionet/PacketExtractor.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS PacketExtractorTests

	namespace {
		uint32_t Default_Max_Packet_Data_Size = 150 * 1024;
		uint32_t Default_Max_Packet_Size = Default_Max_Packet_Data_Size + SizeOf32<PacketHeader>();

		PacketExtractor CreateExtractor(ByteBuffer& buffer, size_t maxPacketDataSize = Default_Max_Packet_Data_Size) {
			return PacketExtractor(buffer, maxPacketDataSize);
		}

		void SetValueAtOffset(ByteBuffer& buffer, size_t offset, uint32_t value) {
			*reinterpret_cast<uint32_t*>(&buffer[offset]) = value;
		}

		void AssertExtractFailure(PacketExtractor& extractor, PacketExtractResult expectedResult) {
			// Act:
			const Packet* pPacket;
			auto result = extractor.tryExtractNextPacket(pPacket);

			// Assert:
			EXPECT_EQ(expectedResult, result);
			EXPECT_FALSE(!!pPacket);
		}

		template<typename TIterator>
		void AssertExtractSuccess(PacketExtractor& extractor, TIterator expectedStart, TIterator expectedEnd) {
			// Act:
			const Packet* pPacket;
			auto result = extractor.tryExtractNextPacket(pPacket);

			// Assert:
			auto pPacketBuffer = reinterpret_cast<const uint8_t*>(pPacket);
			auto packetSize = expectedEnd - expectedStart;
			EXPECT_EQ(PacketExtractResult::Success, result);
			ASSERT_TRUE(!!pPacket);
			EXPECT_TRUE(std::equal(expectedStart, expectedEnd, pPacketBuffer, pPacketBuffer + packetSize));
		}
	}

	namespace {
		void AssertCannotExtractPacketWithIncompleteSize(uint32_t size) {
			// Arrange:
			ByteBuffer buffer(size);
			auto extractor = CreateExtractor(buffer);

			// Assert:
			AssertExtractFailure(extractor, PacketExtractResult::Insufficient_Data);
		}
	}

	TEST(TEST_CLASS, CannotExtractPacketWithIncompleteSize) {
		AssertCannotExtractPacketWithIncompleteSize(0);
		AssertCannotExtractPacketWithIncompleteSize(3);
	}

	namespace {
		void AssertCannotExtractPacketWithSize(uint32_t size, size_t maxPacketDataSize = Default_Max_Packet_Data_Size) {
			// Arrange:
			ByteBuffer buffer(sizeof(PacketHeader));
			SetValueAtOffset(buffer, 0, size);
			auto extractor = CreateExtractor(buffer, maxPacketDataSize);

			// Assert:
			AssertExtractFailure(extractor, PacketExtractResult::Packet_Error);
		}
	}

	TEST(TEST_CLASS, CannotExtractPacketWithSizeLessThanMin) {
		AssertCannotExtractPacketWithSize(sizeof(PacketHeader) - 1);
	}

	TEST(TEST_CLASS, CannotExtractPacketWithSizeGreaterThanMax) {
		AssertCannotExtractPacketWithSize(Default_Max_Packet_Size + 1);
		AssertCannotExtractPacketWithSize(21, 20 - sizeof(PacketHeader));
	}

	namespace {
		void AssertCannotExtractIncompletePacketWithKnownSize(uint32_t size) {
			// Arrange:
			ByteBuffer buffer(4);
			SetValueAtOffset(buffer, 0, size);
			auto extractor = CreateExtractor(buffer);

			// Assert:
			AssertExtractFailure(extractor, PacketExtractResult::Insufficient_Data);
		}
	}

	TEST(TEST_CLASS, CannotExtractIncompletePacketWithKnownSize) {
		AssertCannotExtractIncompletePacketWithKnownSize(sizeof(PacketHeader));
		AssertCannotExtractIncompletePacketWithKnownSize(10);
		AssertCannotExtractIncompletePacketWithKnownSize(Default_Max_Packet_Size);
	}

	namespace {
		void AssertCanExtractCompletePacket(uint32_t packetSize, size_t maxPacketDataSize) {
			// Arrange:
			auto buffer = test::GenerateRandomVector(packetSize);
			SetValueAtOffset(buffer, 0, packetSize);
			auto extractor = CreateExtractor(buffer, maxPacketDataSize);

			// Assert:
			AssertExtractSuccess(extractor, buffer.cbegin(), buffer.cend());
			AssertExtractFailure(extractor, PacketExtractResult::Insufficient_Data);
		}
	}

	TEST(TEST_CLASS, CanExtractCompletePacketWithLessThanMaxSize) {
		AssertCanExtractCompletePacket(19, 20 - sizeof(PacketHeader));
		AssertCanExtractCompletePacket(sizeof(PacketHeader), 20 - sizeof(PacketHeader));
	}

	TEST(TEST_CLASS, CanExtractCompletePacketWithMaxSize) {
		AssertCanExtractCompletePacket(20, 20 - sizeof(PacketHeader));
	}

	TEST(TEST_CLASS, CanExtractMultipleCompletePacketsWithKnownSize) {
		// Arrange:
		ByteBuffer buffer(32);
		SetValueAtOffset(buffer, 0, 20);
		SetValueAtOffset(buffer, 20, 10);
		auto extractor = CreateExtractor(buffer);

		// Assert:
		AssertExtractSuccess(extractor, buffer.cbegin(), buffer.cbegin() + 20);
		AssertExtractSuccess(extractor, buffer.cbegin() + 20, buffer.cbegin() + 30);
		AssertExtractFailure(extractor, PacketExtractResult::Insufficient_Data);
		ASSERT_EQ(32u, buffer.size());
	}

	TEST(TEST_CLASS, CanExtractMultipleCompletePacketsWithKnownSizeInterspersedWithConsumes) {
		// Arrange:
		ByteBuffer buffer(32);
		SetValueAtOffset(buffer, 0, 20);
		SetValueAtOffset(buffer, 20, 10);
		auto extractor = CreateExtractor(buffer);

		// Assert:
		AssertExtractSuccess(extractor, buffer.cbegin(), buffer.cbegin() + 20);
		extractor.consume();
		AssertExtractSuccess(extractor, buffer.cbegin(), buffer.cbegin() + 10);
		extractor.consume();
		AssertExtractFailure(extractor, PacketExtractResult::Insufficient_Data);
		extractor.consume();
		ASSERT_EQ(2u, buffer.size());
	}

	TEST(TEST_CLASS, BufferIsNotConsumedByExtractorWhenConsumeIsNotCalledExplicitly) {
		// Arrange:
		auto buffer = test::GenerateRandomVector(20);
		SetValueAtOffset(buffer, 0, 20);

		// Act:
		{
			auto extractor = CreateExtractor(buffer);
			AssertExtractSuccess(extractor, buffer.cbegin(), buffer.cend());
		}

		// Assert:
		ASSERT_EQ(20u, buffer.size());
	}

	TEST(TEST_CLASS, BufferCanBeCompletelyConsumedByExtractor) {
		// Arrange:
		auto buffer = test::GenerateRandomVector(20);
		SetValueAtOffset(buffer, 0, 20);

		// Act:
		auto extractor = CreateExtractor(buffer);
		AssertExtractSuccess(extractor, buffer.cbegin(), buffer.cend());
		extractor.consume();

		// Assert:
		ASSERT_EQ(0u, buffer.size());
	}

	TEST(TEST_CLASS, BufferCanBePartiallyConsumedByExtractor) {
		// Arrange:
		auto buffer = test::GenerateRandomVector(22);
		SetValueAtOffset(buffer, 0, 20);

		// Act:
		auto extractor = CreateExtractor(buffer);
		AssertExtractSuccess(extractor, buffer.cbegin(), buffer.cbegin() + 20);
		extractor.consume();

		// Assert:
		ASSERT_EQ(2u, buffer.size());
	}

	TEST(TEST_CLASS, ConsumeIsIdempotent) {
		// Arrange:
		auto buffer = test::GenerateRandomVector(22);
		SetValueAtOffset(buffer, 0, 20);

		// Act:
		auto extractor = CreateExtractor(buffer);
		extractor.consume();
		extractor.consume();
		AssertExtractSuccess(extractor, buffer.cbegin(), buffer.cbegin() + 20);
		extractor.consume();
		extractor.consume();

		// Assert:
		ASSERT_EQ(2u, buffer.size());
	}

	TEST(TEST_CLASS, CannotConsumeInsufficientData) {
		// Arrange:
		ByteBuffer buffer(20);
		SetValueAtOffset(buffer, 0, 21);
		auto extractor = CreateExtractor(buffer);

		// Act:
		AssertExtractFailure(extractor, PacketExtractResult::Insufficient_Data);
		extractor.consume();

		// Assert:
		ASSERT_EQ(20u, buffer.size());
	}
}}
