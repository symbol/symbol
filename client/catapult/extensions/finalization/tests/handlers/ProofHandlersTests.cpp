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

#include "finalization/src/handlers/ProofHandlers.h"
#include "finalization/src/api/FinalizationPackets.h"
#include "finalization/src/io/ProofStorageCache.h"
#include "finalization/tests/test/mocks/MockProofStorage.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS ProofHandlersTests

	// region FinalizationStatisticsHandler

	namespace {
		std::unique_ptr<mocks::MockProofStorage> CreateDefaultProofStorage(const Hash256& hash) {
			return std::make_unique<mocks::MockProofStorage>(FinalizationEpoch(3), FinalizationPoint(8), Height(246), hash);
		}
	}

	TEST(TEST_CLASS, FinalizationStatisticsHandler_DoesNotRespondToMalformedRequest) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto proofStorage = io::ProofStorageCache(CreateDefaultProofStorage(hash));

		ionet::ServerPacketHandlers handlers;
		RegisterFinalizationStatisticsHandler(handlers, proofStorage);

		// - malform the packet
		auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
		pPacket->Type = ionet::PacketType::Finalization_Statistics;
		++pPacket->Size;

		// Act:
		ionet::ServerPacketHandlerContext handlerContext;
		EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

		// Assert: malformed packet is ignored
		test::AssertNoResponse(handlerContext);
	}

	TEST(TEST_CLASS, FinalizationStatisticsHandler_WritesFinalizationStatisticsResponseInResponseToValidRequest) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto proofStorage = io::ProofStorageCache(CreateDefaultProofStorage(hash));

		ionet::ServerPacketHandlers handlers;
		RegisterFinalizationStatisticsHandler(handlers, proofStorage);

		// - create a valid request
		auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
		pPacket->Type = ionet::PacketType::Finalization_Statistics;

		// Act:
		ionet::ServerPacketHandlerContext handlerContext;
		EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

		// Assert: finalization statistics are written
		test::AssertPacketHeader(handlerContext, sizeof(api::FinalizationStatisticsResponse), ionet::PacketType::Finalization_Statistics);

		const auto* pResponse = test::GetSingleBufferData(handlerContext);
		const auto* pResponse32 = reinterpret_cast<const uint32_t*>(pResponse);
		const auto* pResponse64 = reinterpret_cast<const uint64_t*>(pResponse);
		EXPECT_EQ(3u, pResponse32[0]); // epoch
		EXPECT_EQ(8u, pResponse32[1]); // point
		EXPECT_EQ(246u, pResponse64[1]); // height
		EXPECT_EQ(hash, reinterpret_cast<const Hash256&>(*(pResponse + 2 * sizeof(uint64_t)))); // hash
	}

	// endregion

	// region FinalizationProofAt(Epoch|Height)Handler

	namespace {
		auto CreateProof() {
			auto pProof = std::make_shared<model::FinalizationProof>();
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pProof.get()), sizeof(model::FinalizationProof) });

			pProof->Size = SizeOf32<model::FinalizationProof>();
			pProof->Round = { FinalizationEpoch(3), FinalizationPoint(11) };
			pProof->Height = Height(246);
			return pProof;
		}

		auto CreateProofStorageCacheWithProof(const std::shared_ptr<model::FinalizationProof>& pProof) {
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto pProofStorage = CreateDefaultProofStorage(hash);
			pProofStorage->setLastFinalizationProof(pProof);
			return io::ProofStorageCache(std::move(pProofStorage));
		}

		struct ProofAtEpochTraits {
			static constexpr auto Register = RegisterFinalizationProofAtEpochHandler;

			static auto CreatePacketWithIdentifier(int64_t delta) {
				auto pPacket = ionet::CreateSharedPacket<api::ProofAtEpochRequest>();
				pPacket->Epoch = FinalizationEpoch(static_cast<uint32_t>(3 + delta));
				return pPacket;
			}

			static auto CreatePacketWithZeroIdentifier() {
				auto pPacket = ionet::CreateSharedPacket<api::ProofAtEpochRequest>();
				pPacket->Epoch = FinalizationEpoch();
				return pPacket;
			}
		};

		struct ProofAtHeightTraits {
			static constexpr auto Register = RegisterFinalizationProofAtHeightHandler;

			static auto CreatePacketWithIdentifier(int64_t delta) {
				auto pPacket = ionet::CreateSharedPacket<api::ProofAtHeightRequest>();
				pPacket->Height = Height(static_cast<uint64_t>(246 + delta));
				return pPacket;
			}

			static auto CreatePacketWithZeroIdentifier() {
				auto pPacket = ionet::CreateSharedPacket<api::ProofAtHeightRequest>();
				pPacket->Height = Height();
				return pPacket;
			}
		};
	}

#define PROOF_AT_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, FinalizationProofAtEpochHandler_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ProofAtEpochTraits>(); } \
	TEST(TEST_CLASS, FinalizationProofAtHeightHandler_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ProofAtHeightTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	PROOF_AT_TEST(DoesNotRespondToMalformedRequest) {
		// Arrange:
		auto proofStorage = CreateProofStorageCacheWithProof(CreateProof());

		ionet::ServerPacketHandlers handlers;
		TTraits::Register(handlers, proofStorage);

		// - create a malformed request
		auto pPacket = TTraits::CreatePacketWithIdentifier(0);
		++pPacket->Size;

		// Act:
		ionet::ServerPacketHandlerContext handlerContext;
		EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

		// Assert: no response was written because the request was malformed
		test::AssertNoResponse(handlerContext);
	}

	namespace {
		template<typename TTraits>
		void AssertWritesEmptyResponse(std::shared_ptr<ionet::Packet>&& pPacket) {
			// Arrange:
			auto proofStorage = CreateProofStorageCacheWithProof(CreateProof());

			ionet::ServerPacketHandlers handlers;
			TTraits::Register(handlers, proofStorage);

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert: only a payload header is written
			test::AssertPacketHeader(handlerContext, sizeof(ionet::PacketHeader), ionet::PacketType::Pull_Finalization_Proof);
			EXPECT_TRUE(handlerContext.response().buffers().empty());
		}
	}

	PROOF_AT_TEST(WritesEmptyResponseWhenRequestHasZeroIdentifier) {
		AssertWritesEmptyResponse<TTraits>(TTraits::CreatePacketWithZeroIdentifier());
	}

	PROOF_AT_TEST(WritesEmptyResponseWhenSpecifiedProofDoesNotExist) {
		AssertWritesEmptyResponse<TTraits>(TTraits::CreatePacketWithIdentifier(-1));
	}

	PROOF_AT_TEST(WritesEmptyResponseWhenSpecifiedProofIsInFuture) {
		AssertWritesEmptyResponse<TTraits>(TTraits::CreatePacketWithIdentifier(1));
	}

	PROOF_AT_TEST(WritesProofWhenSpecifiedProofDoesExist) {
		// Arrange:
		auto pProof = CreateProof();
		auto proofStorage = CreateProofStorageCacheWithProof(pProof);

		ionet::ServerPacketHandlers handlers;
		TTraits::Register(handlers, proofStorage);

		// - create a valid request
		auto pPacket = TTraits::CreatePacketWithIdentifier(0);

		// Act:
		ionet::ServerPacketHandlerContext handlerContext;
		EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

		// Assert: only a payload header is written
		test::AssertPacketHeader(
				handlerContext,
				sizeof(ionet::PacketHeader) + sizeof(model::FinalizationProof),
				ionet::PacketType::Pull_Finalization_Proof);
		EXPECT_EQ_MEMORY(&*pProof, test::GetSingleBufferData(handlerContext), pProof->Size);
	}

	// endregion
}}
