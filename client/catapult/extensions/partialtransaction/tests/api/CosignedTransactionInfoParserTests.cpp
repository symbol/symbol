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

#include "partialtransaction/src/api/CosignedTransactionInfoParser.h"
#include "catapult/ionet/PacketPayloadBuilder.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace api {

#define TEST_CLASS CosignedTransactionInfoParserTests

	namespace {
		constexpr auto Default_Packet_Type = ionet::PacketType::Push_Block; // packet type is not validated by the parser

		auto ExtractCosignedTransactionInfosFromPacket(const ionet::Packet& packet) {
			return api::ExtractCosignedTransactionInfosFromPacket(packet, [](const auto&) { return true; });
		}
	}

#define ASSERT_EQ_COSIGNATURES(EXPECTED, ACTUAL) \
	do { \
		ASSERT_EQ(EXPECTED.size(), ACTUAL.size()); \
		for (auto i = 0u; i < EXPECTED.size(); ++i) { \
			EXPECT_EQ(EXPECTED[i].SignerPublicKey, ACTUAL[i].SignerPublicKey) << "cosignature at " << i; \
			EXPECT_EQ(EXPECTED[i].Signature, ACTUAL[i].Signature) << "cosignature at " << i; \
		} \
	} while (false)

	namespace {
		template<typename TValue>
		void AppendValues(ionet::PacketPayloadBuilder& builder, const std::vector<TValue>& values) {
			for (const auto& value : values)
				builder.appendValue(value);
		}

		std::shared_ptr<ionet::Packet> BuildPacket(ionet::PacketPayloadBuilder& builder) {
			auto payload = builder.build();
			size_t dataSize = 0u;
			for (const auto& buffer : payload.buffers())
				dataSize += buffer.Size;

			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(static_cast<uint32_t>(dataSize));
			auto* pData = pPacket->Data();
			for (const auto& buffer : payload.buffers()) {
				std::memcpy(pData, buffer.pData, buffer.Size);
				pData += buffer.Size;
			}

			CATAPULT_LOG(debug) << "generated " << payload.header() << " from " << payload.buffers().size() << " buffers";
			return pPacket;
		}

		// region generators

		class HashGenerator {
		public:
			explicit HashGenerator(uint8_t numCosignatures)
					: m_numCosignatures(numCosignatures)
					, m_hash(test::GenerateRandomByteArray<Hash256>())
					, m_cosignatures(test::GenerateRandomDataVector<model::Cosignature>(numCosignatures))
			{}

		public:
			void addData(ionet::PacketPayloadBuilder& builder) const {
				builder.appendValue<uint64_t>(0x0000 | m_numCosignatures);
				builder.appendValue(m_hash);
				AppendValues(builder, m_cosignatures);
			}

			void assertInfo(const model::CosignedTransactionInfo& transactionInfo) const {
				EXPECT_EQ(m_hash, transactionInfo.EntityHash);
				EXPECT_FALSE(!!transactionInfo.pTransaction);
				ASSERT_EQ_COSIGNATURES(m_cosignatures, transactionInfo.Cosignatures);
			}

		private:
			uint8_t m_numCosignatures;
			Hash256 m_hash;
			std::vector<model::Cosignature> m_cosignatures;
		};

		class TransactionGenerator {
		public:
			explicit TransactionGenerator(uint8_t numCosignatures)
					: m_numCosignatures(numCosignatures)
					, m_pTransaction(test::GenerateRandomTransaction())
					, m_cosignatures(test::GenerateRandomDataVector<model::Cosignature>(numCosignatures))
			{}

		public:
			void addData(ionet::PacketPayloadBuilder& builder) const {
				builder.appendValue<uint64_t>(0x8000 | m_numCosignatures);
				builder.appendEntity(m_pTransaction);
				builder.appendValues(std::vector<uint8_t>(utils::GetPaddingSize(m_pTransaction->Size, 8), 0));
				AppendValues(builder, m_cosignatures);
			}

			void assertInfo(const model::CosignedTransactionInfo& transactionInfo) const {
				EXPECT_EQ(Hash256(), transactionInfo.EntityHash);
				ASSERT_TRUE(!!transactionInfo.pTransaction);
				EXPECT_EQ(*m_pTransaction, *transactionInfo.pTransaction);
				ASSERT_EQ_COSIGNATURES(m_cosignatures, transactionInfo.Cosignatures);
			}

		private:
			uint8_t m_numCosignatures;
			std::shared_ptr<model::Transaction> m_pTransaction;
			std::vector<model::Cosignature> m_cosignatures;
		};

		// endregion
	}

	// region valid parses

	TEST(TEST_CLASS, NoEntriesAreExtractedFromEmptyPacket) {
		// Arrange:
		ionet::PacketPayloadBuilder builder(Default_Packet_Type);
		auto pPacket = BuildPacket(builder);

		// Act:
		auto extractedInfos = ExtractCosignedTransactionInfosFromPacket(*pPacket);

		// Assert:
		EXPECT_TRUE(extractedInfos.empty());
	}

	// endregion

	// region single info (all permutations)

	namespace {
		template<typename TGenerator>
		void AssertSingleEntryParse(const TGenerator& generator) {
			// Arrange:
			ionet::PacketPayloadBuilder builder(Default_Packet_Type);
			generator.addData(builder);
			auto pPacket = BuildPacket(builder);

			// Act:
			auto extractedInfos = ExtractCosignedTransactionInfosFromPacket(*pPacket);

			// Assert:
			ASSERT_EQ(1u, extractedInfos.size());
			generator.assertInfo(extractedInfos[0]);
		}
	}

	TEST(TEST_CLASS, CanExtractHashOnlyEntry) {
		AssertSingleEntryParse(HashGenerator(0));
	}

	TEST(TEST_CLASS, CanExtractHashAndCosignaturesEntry) {
		AssertSingleEntryParse(HashGenerator(3));
	}

	TEST(TEST_CLASS, CanExtractTransactionOnlyEntry) {
		AssertSingleEntryParse(TransactionGenerator(0));
	}

	TEST(TEST_CLASS, CanExtractTransactionAndCosignaturesEntry) {
		AssertSingleEntryParse(TransactionGenerator(3));
	}

	// endregion

	// region multiple infos

	TEST(TEST_CLASS, CanExtractMultipleHeterogeneousEntries) {
		// Arrange:
		auto generator1 = HashGenerator(0);
		auto generator2 = TransactionGenerator(3);
		auto generator3 = HashGenerator(3);
		auto generator4 = TransactionGenerator(0);

		ionet::PacketPayloadBuilder builder(Default_Packet_Type);
		generator1.addData(builder);
		generator2.addData(builder);
		generator3.addData(builder);
		generator4.addData(builder);
		auto pPacket = BuildPacket(builder);

		// Act:
		auto extractedInfos = ExtractCosignedTransactionInfosFromPacket(*pPacket);

		// Assert:
		ASSERT_EQ(4u, extractedInfos.size());
		generator1.assertInfo(extractedInfos[0]);
		generator2.assertInfo(extractedInfos[1]);
		generator3.assertInfo(extractedInfos[2]);
		generator4.assertInfo(extractedInfos[3]);
	}

	// endregion

	// region insufficient / invalid data (notice that all cause failure of *second* entry)

	namespace {
		template<typename TCorruptPayload>
		void AssertParseFailure(TCorruptPayload corruptPayload) {
			// Arrange:
			auto generator = TransactionGenerator(2);

			// - corrupt the payload after a valid entry
			ionet::PacketPayloadBuilder builder(Default_Packet_Type);
			generator.addData(builder);
			corruptPayload(builder);
			auto pPacket = BuildPacket(builder);

			// Act:
			auto extractedInfos = ExtractCosignedTransactionInfosFromPacket(*pPacket);

			// Assert:
			EXPECT_TRUE(extractedInfos.empty());
		}
	}

	TEST(TEST_CLASS, FailureWhenTagCannotBeExtracted) {
		AssertParseFailure([](auto& builder) {
			// Arrange: only partial tag is written (tag is uint16_t)
			builder.appendValue(static_cast<uint8_t>(0x00));
		});
	}

	TEST(TEST_CLASS, FailureWhenTagIsFollowedByInsufficientPadding) {
		AssertParseFailure([](auto& builder) {
			// Arrange: only 2 pad bytes are present (6 expected)
			builder.appendValue(static_cast<uint32_t>(0x0000'0000));
		});
	}

	TEST(TEST_CLASS, FailureWhenTagIsFollowedByNonzeroPadding) {
		AssertParseFailure([](auto& builder) {
			// Arrange: bit is set in reserved tag area
			builder.appendValue(static_cast<uint64_t>(0x0200'0000'0000'0000));
		});
	}

	TEST(TEST_CLASS, FailureWhenHashCannotBeExtracted) {
		AssertParseFailure([](auto& builder) {
			// Arrange: hash is expected but not present
			builder.appendValue(static_cast<uint64_t>(0x0000));
		});
	}

	TEST(TEST_CLASS, FailureWhenTransactionCannotBeExtracted) {
		AssertParseFailure([](auto& builder) {
			// Arrange: transaction is expected but not present
			builder.appendValue(static_cast<uint64_t>(0x8000));
		});
	}

	TEST(TEST_CLASS, FailureWhenTransactionDataCannotBeExtracted) {
		AssertParseFailure([](auto& builder) {
			// Arrange: transaction is expected but only header (size) is present
			builder.appendValue(static_cast<uint64_t>(0x8000));
			builder.appendValue(SizeOf32<model::Transaction>());
		});
	}

	TEST(TEST_CLASS, FailureWhenTransactionIsFollowedByInsufficientPadding) {
		AssertParseFailure([](auto& builder) {
			// Arrange: only 2 pad bytes are present (4 expected)
			auto pTransaction = utils::UniqueToShared(test::GenerateRandomTransactionWithSize(140));
			builder.template appendValue<uint64_t>(0x8000);
			builder.appendEntity(pTransaction);
			builder.template appendValue<uint16_t>(0x0000);
		});
	}

	TEST(TEST_CLASS, FailureWhenTransactionIsFollowedByNonzeroPadding) {
		AssertParseFailure([](auto& builder) {
			// Arrange: bit is set in reserved tag area
			auto pTransaction = utils::UniqueToShared(test::GenerateRandomTransactionWithSize(140));
			builder.template appendValue<uint64_t>(0x8000);
			builder.appendEntity(pTransaction);
			builder.template appendValue<uint32_t>(0x0020'0000);
		});
	}

	TEST(TEST_CLASS, FailureWhenCosignatureCannotBeExtracted) {
		AssertParseFailure([](auto& builder) {
			// Arrange: two cosignatures are present but only one is written
			builder.appendValue(static_cast<uint64_t>(0x0002));
			builder.appendValue(test::GenerateRandomByteArray<Hash256>());
			AppendValues(builder, test::GenerateRandomDataVector<model::Cosignature>(1));
		});
	}

	TEST(TEST_CLASS, FailureWhenTransactionFailsValidation) {
		// Arrange:
		auto generator1 = TransactionGenerator(2);
		auto generator2 = TransactionGenerator(1);

		ionet::PacketPayloadBuilder builder(Default_Packet_Type);
		generator1.addData(builder);
		generator2.addData(builder);
		auto pPacket = BuildPacket(builder);

		// Act: parse and indicate the second transaction is invalid
		auto i = 0u;
		auto extractedInfos = api::ExtractCosignedTransactionInfosFromPacket(*pPacket, [&i](const auto&) {
			return 2u == ++i;
		});

		// Assert:
		EXPECT_TRUE(extractedInfos.empty());
	}

	// endregion
}}
