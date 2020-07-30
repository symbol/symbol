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

#include "src/parsers/TransactionElementParser.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace parsers {

#define TEST_CLASS TransactionElementParserTests

	namespace {
		constexpr auto Transaction_Size = SizeOf32<mocks::MockTransaction>();
		constexpr auto Hash_Size = static_cast<uint32_t>(Hash256::Size);
	}

	// region TryParseTransactionElements

	namespace {
		void AssertCannotParsePacketWithSize(uint32_t size) {
			// Arrange: override the packet size
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(sizeof(uint32_t));
			pPacket->Size = size;

			// Act:
			std::vector<model::TransactionElement> elements;
			auto result = TryParseTransactionElements(*pPacket, test::DefaultSizeCheck<model::Transaction>, elements);

			// Assert:
			EXPECT_FALSE(result) << "with size " << size;
		}
	}

	TEST(TEST_CLASS, CannotParsePacketWithoutFullHeader) {
		// Assert: packet must contain packet header and sub header
		AssertCannotParsePacketWithSize(sizeof(ionet::PacketHeader) - 1);
		AssertCannotParsePacketWithSize(sizeof(ionet::PacketHeader));
		AssertCannotParsePacketWithSize(sizeof(ionet::PacketHeader) + sizeof(uint32_t) - 1);
	}

	namespace {
		void AssertCannotParsePacketWithInvalidHashes(uint32_t hashesSize, uint32_t reportedHashesSize) {
			// Arrange: set the reported packet size to be invalid
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(SizeOf32<uint32_t>() + hashesSize);
			reinterpret_cast<uint32_t&>(*pPacket->Data()) = reportedHashesSize;

			// Act:
			std::vector<model::TransactionElement> elements;
			auto result = TryParseTransactionElements(*pPacket, test::DefaultSizeCheck<model::Transaction>, elements);

			// Assert:
			EXPECT_FALSE(result) << "with hashes size " << hashesSize << " and reported hashes size " << reportedHashesSize;
		}
	}

	TEST(TEST_CLASS, CannotParsePacketWithoutCompleteDataPayload) {
		AssertCannotParsePacketWithInvalidHashes(1, 2);
		AssertCannotParsePacketWithInvalidHashes(1, 100);
		AssertCannotParsePacketWithInvalidHashes(100, 101);
	}

	TEST(TEST_CLASS, CannotParsePacketWithoutHashes) {
		AssertCannotParsePacketWithInvalidHashes(0, 0);
	}

	TEST(TEST_CLASS, CannotParsePacketWithInvalidHashes) {
		// Assert: data must be evenly divisible
		for (auto size : std::initializer_list<uint32_t>{ Hash256::Size - 1, 3 * Hash256::Size - 1 })
			AssertCannotParsePacketWithInvalidHashes(size, size);
	}

	TEST(TEST_CLASS, CannotParsePacketWithoutEntities) {
		AssertCannotParsePacketWithInvalidHashes(2 * Hash256::Size, 2 * Hash256::Size);
	}

	namespace {
		auto CreatePacketWithHashesAndEntities(uint32_t numHashes, uint32_t numEntities) {
			uint32_t payloadSize = SizeOf32<uint32_t>() + numHashes * Hash_Size + numEntities * Transaction_Size;
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
			test::FillWithRandomData({ pPacket->Data(), pPacket->Size - sizeof(ionet::PacketHeader) });
			reinterpret_cast<uint32_t&>(*pPacket->Data()) = numHashes * Hash_Size;

			// - set entity sizes
			for (auto i = 0u; i < numEntities; ++i) {
				auto entityOffset = sizeof(uint32_t) + numHashes * Hash256::Size + i * Transaction_Size;
				auto& transaction = reinterpret_cast<mocks::MockTransaction&>(*(pPacket->Data() + entityOffset));
				transaction.Size = Transaction_Size;
				transaction.Type = mocks::MockTransaction::Entity_Type;
				transaction.Data.Size = 0;
			}

			return pPacket;
		}
	}

	TEST(TEST_CLASS, CannotParsePacketWithInvalidEntities) {
		// Arrange:
		auto pPacket = CreatePacketWithHashesAndEntities(2, 1);

		// Act:
		auto numValidCalls = 0u;
		std::vector<model::TransactionElement> elements;
		auto result = TryParseTransactionElements(*pPacket, [&numValidCalls](const auto&) {
			++numValidCalls;
			return false;
		}, elements);

		// Assert:
		EXPECT_EQ(1u, numValidCalls);
		EXPECT_FALSE(result);
	}

	namespace {
		void AssertCannotParsePacketWithWrongNumberOfEntities(uint32_t numHashes, uint32_t numEntities) {
			// Arrange:
			auto pPacket = CreatePacketWithHashesAndEntities(numHashes, numEntities);

			// Act:
			std::vector<model::TransactionElement> elements;
			auto result = TryParseTransactionElements(*pPacket, test::DefaultSizeCheck<model::Transaction>, elements);

			// Assert:
			EXPECT_FALSE(result) << "with hashes " << numHashes << " and entities " << numEntities;
		}
	}

	TEST(TEST_CLASS, CannotParsePacketWithWrongNumberOfEntities) {
		AssertCannotParsePacketWithWrongNumberOfEntities(1, 2);
		AssertCannotParsePacketWithWrongNumberOfEntities(2, 3);
		AssertCannotParsePacketWithWrongNumberOfEntities(2, 5);
		AssertCannotParsePacketWithWrongNumberOfEntities(10, 1);
		AssertCannotParsePacketWithWrongNumberOfEntities(1, 10);
	}

	namespace {
		void AssertCanParsePacketWithEntities(uint32_t numEntities) {
			// Arrange:
			auto pPacket = CreatePacketWithHashesAndEntities(2 * numEntities, numEntities);

			// Act:
			std::vector<model::TransactionElement> elements;
			auto result = TryParseTransactionElements(*pPacket, test::DefaultSizeCheck<model::Transaction>, elements);

			// Assert:
			EXPECT_TRUE(result);
			ASSERT_EQ(numEntities, elements.size());

			const auto* pHash = reinterpret_cast<const Hash256*>(pPacket->Data() + sizeof(uint32_t));
			const auto* pTransaction = reinterpret_cast<const mocks::MockTransaction*>(pHash + 2 * numEntities);
			auto i = 0u;
			for (const auto& element : elements) {
				auto message = "element at " + std::to_string(i++);
				EXPECT_EQ(pTransaction, &element.Transaction) << message;
				++pTransaction;
				EXPECT_EQ(*pHash, element.EntityHash) << message;
				++pHash;
				EXPECT_EQ(*pHash, element.MerkleComponentHash) << message;
				++pHash;
			}
		}
	}

	TEST(TEST_CLASS, CanParsePacketWithSingleEntity) {
		AssertCanParsePacketWithEntities(1);
	}

	TEST(TEST_CLASS, CanParsePacketWithMultipleEntities) {
		AssertCanParsePacketWithEntities(3);
	}

	// endregion
}}
