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

#include "finalization/src/api/RemoteProofApi.h"
#include "finalization/src/api/FinalizationPackets.h"
#include "tests/test/other/RemoteApiFactory.h"
#include "tests/test/other/RemoteApiTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace api {

	namespace {
		struct FinalizationStatisticsTraits {
			static auto Invoke(const RemoteProofApi& api) {
				return api.finalizationStatistics();
			}

			static auto CreateValidResponsePacket() {
				auto pResponsePacket = ionet::CreateSharedPacket<FinalizationStatisticsResponse>();
				pResponsePacket->Point = FinalizationPoint(1234);
				pResponsePacket->Height = Height(625);
				pResponsePacket->Hash = { { 32 } };
				return pResponsePacket;
			}

			static auto CreateMalformedResponsePacket() {
				// just change the size because no responses are intrinsically invalid
				auto pResponsePacket = CreateValidResponsePacket();
				--pResponsePacket->Size;
				return pResponsePacket;
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				EXPECT_TRUE(ionet::IsPacketValid(packet, FinalizationStatisticsResponse::Packet_Type));
			}

			static void ValidateResponse(const ionet::Packet&, const model::FinalizationStatistics& finalizationStatistics) {
				EXPECT_EQ(FinalizationPoint(1234), finalizationStatistics.Point);
				EXPECT_EQ(Height(625), finalizationStatistics.Height);
				EXPECT_EQ(Hash256{ { 32 } }, finalizationStatistics.Hash);
			}
		};

		struct ProofAtPointInvoker {
			static auto Invoke(const RemoteProofApi& api) {
				return api.proofAt(FinalizationPoint(123));
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				const auto* pRequest = ionet::CoercePacket<ProofAtPointRequest>(&packet);
				ASSERT_TRUE(!!pRequest);
				EXPECT_EQ(FinalizationPoint(123), pRequest->Point);
			}
		};

		struct ProofAtHeightInvoker {
			static auto Invoke(const RemoteProofApi& api) {
				return api.proofAt(Height(728));
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				const auto* pRequest = ionet::CoercePacket<ProofAtHeightRequest>(&packet);
				ASSERT_TRUE(!!pRequest);
				EXPECT_EQ(Height(728), pRequest->Height);
			}
		};

		template<typename TInvoker>
		struct ProofAtTraitsT : public TInvoker {
			static auto CreateValidResponsePacket() {
				uint32_t payloadSize = SizeOf32<model::FinalizationProof>();
				auto pResponsePacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
				pResponsePacket->Type = ionet::PacketType::Pull_Finalization_Proof;
				test::FillWithRandomData({ pResponsePacket->Data(), payloadSize });

				auto& proof = reinterpret_cast<model::FinalizationProof&>(*pResponsePacket->Data());
				proof.Size = payloadSize;
				proof.Height = Height(777);
				return pResponsePacket;
			}

			static auto CreateMalformedResponsePacket() {
				// the packet is malformed because it contains a partial proof
				auto pResponsePacket = CreateValidResponsePacket();
				--pResponsePacket->Size;
				return pResponsePacket;
			}

			static void ValidateResponse(const ionet::Packet& response, const std::shared_ptr<const model::FinalizationProof>& pProof) {
				ASSERT_EQ(response.Size - sizeof(ionet::Packet), pProof->Size);
				ASSERT_EQ(sizeof(model::FinalizationProof), pProof->Size);
				EXPECT_EQ(Height(777), pProof->Height);
				EXPECT_EQ_MEMORY(response.Data(), pProof.get(), pProof->Size);
			}
		};

		using ProofAtPointTraits = ProofAtTraitsT<ProofAtPointInvoker>;
		using ProofAtHeightTraits = ProofAtTraitsT<ProofAtHeightInvoker>;

		struct RemoteProofApiTraits {
			static auto Create(ionet::PacketIo& packetIo, const model::NodeIdentity& remoteIdentity) {
				return CreateRemoteProofApi(packetIo, remoteIdentity);
			}

			static auto Create(ionet::PacketIo& packetIo) {
				return Create(packetIo, model::NodeIdentity());
			}
		};
	}

	DEFINE_REMOTE_API_TESTS(RemoteProofApi)
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteProofApi, FinalizationStatistics)
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteProofApi, ProofAtPoint)
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteProofApi, ProofAtHeight)
}}
