#include "src/extensions/RemoteDiagnosticApi.h"
#include "catapult/handlers/DiagnosticHandlers.h"
#include "catapult/ionet/Packet.h"
#include "catapult/model/AccountInfo.h"
#include "tests/test/other/RemoteApiTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

	namespace {

		// region AccountInfosTraits

		struct AccountInfosTraits {
			using RequestParamType = model::AddressRange;
			using ResponseType = model::AccountInfoRange;

			static constexpr auto PacketType() { return ionet::PacketType::Account_Infos; }
			static constexpr auto Request_Entity_Size = Address_Decoded_Size;
			static constexpr auto Response_Entity_Size = sizeof(model::AccountInfo) + sizeof(model::Mosaic);

			static auto CreateResponsePacket(uint32_t numAccountInfos) {
				uint32_t payloadSize = numAccountInfos * Response_Entity_Size;
				auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
				test::FillWithRandomData({ pPacket->Data(), payloadSize });

				auto pData = pPacket->Data();
				for (auto i = 0u; i < numAccountInfos; ++i, pData += Response_Entity_Size) {
					auto& accountInfo = reinterpret_cast<model::AccountInfo&>(*pData);
					accountInfo.Size = Response_Entity_Size;
					accountInfo.AddressHeight = Height(5 * i);
					accountInfo.MosaicsCount = 1u;
					auto pMosaic = accountInfo.MosaicsPtr();
					pMosaic->MosaicId = Xem_Id;
					pMosaic->Amount = Amount(123 * i);
				}

				return pPacket;
			}

			static std::vector<Address> RequestParamValues() {
				return { { { 123 } }, { { 234 } }, { { 213 } } };
			}

			static auto Invoke(const RemoteDiagnosticApi& api, RequestParamType&& param) {
				return api.accountInfos(std::move(param));
			}

			static void ValidateResponse(const ionet::Packet& response, const model::AccountInfoRange& accountInfos) {
				ASSERT_EQ(3u, accountInfos.size());

				auto pData = response.Data();
				auto iter = accountInfos.cbegin();
				for (auto i = 0u; i < accountInfos.size(); ++i) {
					std::string message = "comparing account info at " + std::to_string(i);
					const auto& expectedAccountInfo = reinterpret_cast<const model::AccountInfo&>(*pData);
					const auto& actualAccountInfo = *iter;
					ASSERT_EQ(expectedAccountInfo.Size, actualAccountInfo.Size) << message;
					EXPECT_EQ(Height(5 * i), actualAccountInfo.AddressHeight) << message;

					const auto& expectedMosaic = *expectedAccountInfo.MosaicsPtr();
					const auto& actualMosaic = *actualAccountInfo.MosaicsPtr();
					EXPECT_EQ(expectedMosaic.MosaicId, actualMosaic.MosaicId) << message;
					EXPECT_EQ(expectedMosaic.Amount, actualMosaic.Amount) << message;
					++iter;
					pData += expectedAccountInfo.Size;
				}
			}
		};

		// endregion

		// region ConfirmTimestampedHashesTraits

		struct ConfirmTimestampedHashesTraits {
			using RequestParamType = state::TimestampedHashRange;
			using ResponseType = state::TimestampedHashRange;

			static constexpr auto PacketType() { return ionet::PacketType::Confirm_Timestamped_Hashes; }
			static constexpr auto Request_Entity_Size = sizeof(state::TimestampedHash);
			static constexpr auto Response_Entity_Size = Request_Entity_Size;

			static auto CreateResponsePacket(uint32_t numTimestampedHashes) {
				uint32_t payloadSize = numTimestampedHashes * Response_Entity_Size;
				auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
				test::FillWithRandomData({ pPacket->Data(), payloadSize });

				auto pData = pPacket->Data();
				for (auto i = 0u; i < numTimestampedHashes; ++i, pData += Response_Entity_Size) {
					auto& timestampedHash = reinterpret_cast<state::TimestampedHash&>(*pData);
					timestampedHash.Time = Timestamp(5 * i);
					test::FillWithRandomData(timestampedHash.Hash);
				}

				return pPacket;
			}

			static std::vector<state::TimestampedHash> RequestParamValues() {
				return {
					state::TimestampedHash(Timestamp(12), { { 123 } }),
					state::TimestampedHash(Timestamp(23), { { 234 } }),
					state::TimestampedHash(Timestamp(34), { { 213 } })
				};
			}

			static auto Invoke(const RemoteDiagnosticApi& api, RequestParamType&& param) {
				return api.confirmTimestampedHashes(std::move(param));
			}

			static void ValidateResponse(const ionet::Packet& response, const state::TimestampedHashRange& timestampedHashes) {
				ASSERT_EQ(3u, timestampedHashes.size());

				auto pExpectedTimestampedHash = reinterpret_cast<const state::TimestampedHash*>(response.Data());
				auto iter = timestampedHashes.cbegin();
				for (auto i = 0u; i < timestampedHashes.size(); ++i) {
					std::string message = "comparing timestamped hash at " + std::to_string(i);
					const auto& actualTimestampedHash = *iter;
					EXPECT_EQ(*pExpectedTimestampedHash, actualTimestampedHash) << message;
					++pExpectedTimestampedHash;
					++iter;
				}
			}
		};

		// endregion

		// region DiagnosticCountersTraits

		struct DiagnosticCountersTraits {
			static constexpr auto Packet_Type = ionet::PacketType::Diagnostic_Counters;

			static auto Invoke(const RemoteDiagnosticApi& api) {
				return api.diagnosticCounters();
			}

			static auto CreateValidResponsePacket() {
				auto pResponsePacket = ionet::CreateSharedPacket<ionet::Packet>(3 * sizeof(model::DiagnosticCounterValue));
				pResponsePacket->Type = Packet_Type;

				auto* pCounters = reinterpret_cast<model::DiagnosticCounterValue*>(pResponsePacket->Data());
				pCounters[0] = { 123u, 7u };
				pCounters[1] = { 777u, 88u };
				pCounters[2] = { 225u, 22u };
				return pResponsePacket;
			}

			static auto CreateMalformedResponsePacket() {
				// just change the size because no responses are intrinsically invalid
				auto pResponsePacket = CreateValidResponsePacket();
				--pResponsePacket->Size;
				return pResponsePacket;
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				EXPECT_TRUE(ionet::IsPacketValid(packet, Packet_Type));
			}

			static void ValidateResponse(const ionet::Packet&, const model::EntityRange<model::DiagnosticCounterValue>& counterValues) {
				ASSERT_EQ(3u, counterValues.size());

				auto iter = counterValues.cbegin();
				EXPECT_EQ(123u, iter->Id);
				EXPECT_EQ(7u, iter->Value);

				++iter;
				EXPECT_EQ(777u, iter->Id);
				EXPECT_EQ(88u, iter->Value);

				++iter;
				EXPECT_EQ(225u, iter->Id);
				EXPECT_EQ(22u, iter->Value);
			}
		};

		// endregion

		// region ActiveNodeInfosTraits

		struct ActiveNodeInfosTraits {
			static constexpr auto Packet_Type = ionet::PacketType::Active_Node_Infos;
			static constexpr auto Response_Entity_Size = sizeof(ionet::PackedNodeInfo) + 2 * sizeof(ionet::PackedConnectionState);
			static constexpr auto Num_Node_Infos = 3u;

			static auto Invoke(const RemoteDiagnosticApi& api) {
				return api.activeNodeInfos();
			}

			static auto CreateValidResponsePacket() {
				uint32_t payloadSize = Num_Node_Infos * Response_Entity_Size;
				auto pResponsePacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
				pResponsePacket->Type = Packet_Type;
				test::FillWithRandomData({ pResponsePacket->Data(), payloadSize });

				// set sizes appropriately
				auto* pData = pResponsePacket->Data();
				for (auto i = 0u; i < Num_Node_Infos; ++i) {
					auto& nodeInfo = reinterpret_cast<ionet::PackedNodeInfo&>(*pData);
					nodeInfo.Size = Response_Entity_Size;
					nodeInfo.ConnectionStatesCount = 2;
					pData += Response_Entity_Size;
				}

				return pResponsePacket;
			}

			static auto CreateMalformedResponsePacket() {
				// just change the size because no responses are intrinsically invalid
				auto pResponsePacket = CreateValidResponsePacket();
				--pResponsePacket->Size;
				return pResponsePacket;
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				EXPECT_TRUE(ionet::IsPacketValid(packet, Packet_Type));
			}

			static void ValidateResponse(const ionet::Packet& response, const model::EntityRange<ionet::PackedNodeInfo>& nodeInfos) {
				ASSERT_EQ(static_cast<uint32_t>(Num_Node_Infos), nodeInfos.size());

				auto iter = nodeInfos.cbegin();
				const auto* pResponseData = response.Data();
				for (auto i = 0u; i < Num_Node_Infos; ++i) {
					auto expectedSize = Response_Entity_Size;
					auto message = "node info at " + std::to_string(i);

					// Sanity:
					ASSERT_EQ(expectedSize, reinterpret_cast<const ionet::PackedNodeInfo&>(*pResponseData).Size) << message;

					// Assert: check the node info size then the memory
					ASSERT_EQ(expectedSize, iter->Size) << message;
					EXPECT_TRUE(0 == memcmp(pResponseData, &*iter, iter->Size)) << message;

					pResponseData += expectedSize;
					++iter;
				}
			}
		};

		// endregion

		// region NamespaceInfosTraits

		struct NamespaceInfosTraits {
			using RequestParamType = model::EntityRange<NamespaceId>;
			using ResponseType = model::EntityRange<model::NamespaceInfo>;

			static constexpr auto PacketType() { return ionet::PacketType::Namespace_Infos; }
			static constexpr auto Request_Entity_Size = sizeof(NamespaceId);
			static constexpr auto Response_Entity_Size = sizeof(model::NamespaceInfo) + 3 * sizeof(NamespaceId);

			static auto CreateResponsePacket(uint32_t numNamespaceInfos) {
				uint32_t payloadSize = numNamespaceInfos * Response_Entity_Size;
				auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
				test::FillWithRandomData({ pPacket->Data(), payloadSize });

				auto pData = pPacket->Data();
				for (auto i = 0u; i < numNamespaceInfos; ++i, pData += Response_Entity_Size) {
					auto& namespaceInfo = reinterpret_cast<model::NamespaceInfo&>(*pData);
					namespaceInfo.Size = Response_Entity_Size;
					namespaceInfo.Id = NamespaceId(5 * i);
					namespaceInfo.ChildCount = 3;
					auto* pChildId = namespaceInfo.ChildrenPtr();
					*pChildId++ = NamespaceId(5 * i + 1);
					*pChildId++ = NamespaceId(5 * i + 2);
					*pChildId = NamespaceId(5 * i + 3);
				}

				return pPacket;
			}

			static std::vector<NamespaceId> RequestParamValues() {
				return { NamespaceId(123), NamespaceId(234), NamespaceId(213) };
			}

			static auto Invoke(const RemoteDiagnosticApi& api, RequestParamType&& param) {
				return api.namespaceInfos(std::move(param));
			}

			static void ValidateResponse(const ionet::Packet& response, const model::EntityRange<model::NamespaceInfo>& namespaceInfos) {
				ASSERT_EQ(3u, namespaceInfos.size());

				auto pData = response.Data();
				auto iter = namespaceInfos.cbegin();
				for (auto i = 0u; i < namespaceInfos.size(); ++i) {
					std::string message = "comparing namespace info at " + std::to_string(i);
					const auto& expectedNamespaceInfo = reinterpret_cast<const model::NamespaceInfo&>(*pData);
					const auto& actualNamespaceInfo = *iter;
					ASSERT_EQ(expectedNamespaceInfo.Size, actualNamespaceInfo.Size) << message;
					EXPECT_EQ(NamespaceId(5 * i), actualNamespaceInfo.Id) << message;

					const auto* pExpectedChildId = expectedNamespaceInfo.ChildrenPtr();
					const auto* pActualChildId = actualNamespaceInfo.ChildrenPtr();
					for (auto j = 0; j < 3; ++j) {
						EXPECT_EQ(*pExpectedChildId, *pActualChildId) << message << " child at " << j;
						++pExpectedChildId;
						++pActualChildId;
					}

					++iter;
					pData += expectedNamespaceInfo.Size;
				}
			}
		};

		// endregion

		// region MosaicInfosTraits

		struct MosaicInfosTraits {
			using RequestParamType = model::EntityRange<MosaicId>;
			using ResponseType = model::EntityRange<model::MosaicInfo>;

			static constexpr auto PacketType() { return ionet::PacketType::Mosaic_Infos; }
			static constexpr auto Request_Entity_Size = sizeof(MosaicId);
			static constexpr auto Response_Entity_Size = sizeof(model::MosaicInfo);

			static auto CreateResponsePacket(uint32_t numMosaicInfos) {
				uint32_t payloadSize = numMosaicInfos * Response_Entity_Size;
				auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
				test::FillWithRandomData({ pPacket->Data(), payloadSize });

				auto pData = pPacket->Data();
				for (auto i = 0u; i < numMosaicInfos; ++i, pData += Response_Entity_Size) {
					auto& mosaicInfo = reinterpret_cast<model::MosaicInfo&>(*pData);
					mosaicInfo.Id = MosaicId(5 * i);
				}

				return pPacket;
			}

			static std::vector<MosaicId> RequestParamValues() {
				return { MosaicId(123), MosaicId(234), MosaicId(213) };
			}

			static auto Invoke(const RemoteDiagnosticApi& api, RequestParamType&& param) {
				return api.mosaicInfos(std::move(param));
			}

			static void ValidateResponse(const ionet::Packet&, const model::EntityRange<model::MosaicInfo>& mosaicInfos) {
				ASSERT_EQ(3u, mosaicInfos.size());

				auto iter = mosaicInfos.cbegin();
				for (auto i = 0u; i < mosaicInfos.size(); ++i) {
					const auto& actualMosaicInfo = *iter;
					EXPECT_EQ(MosaicId(5 * i), actualMosaicInfo.Id) << "comparing mosaic info at " << i;
					++iter;
				}
			}
		};

		// endregion

		// region DiagnosticApiTraits

		template<typename TTraits>
		struct DiagnosticApiTraits {
			static constexpr uint32_t Num_Entities = 3;
			static constexpr uint32_t Request_Data_Size = Num_Entities * TTraits::Request_Entity_Size;

			static auto CreateRequestParam() {
				return TTraits::RequestParamType::CopyFixed(
						reinterpret_cast<uint8_t*>(TTraits::RequestParamValues().data()),
						Num_Entities);
			}

			static auto Invoke(const RemoteDiagnosticApi& api) {
				return TTraits::Invoke(api, CreateRequestParam());
			}

			static auto CreateValidResponsePacket() {
				auto pResponsePacket = TTraits::CreateResponsePacket(Num_Entities);
				pResponsePacket->Type = TTraits::PacketType();
				return pResponsePacket;
			}

			static auto CreateMalformedResponsePacket() {
				// the packet is malformed because it contains a partial entity
				auto pResponsePacket = CreateValidResponsePacket();
				--pResponsePacket->Size;
				return pResponsePacket;
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				EXPECT_EQ(TTraits::PacketType(), packet.Type);
				EXPECT_EQ(sizeof(ionet::Packet) + Request_Data_Size, packet.Size);
				EXPECT_TRUE(0 == std::memcmp(packet.Data(), TTraits::RequestParamValues().data(), Request_Data_Size));
			}

			static void ValidateResponse(const ionet::Packet& response, const typename TTraits::ResponseType& responseEntities) {
				TTraits::ValidateResponse(response, responseEntities);
			}
		};

		// endregion

		// region RemoteDiagnosticApiTraits

		struct RemoteDiagnosticApiTraits {
			static auto Create(const std::shared_ptr<ionet::PacketIo>& pPacketIo) {
				return CreateRemoteDiagnosticApi(*pPacketIo);
			}
		};

		// endregion
	}

	using DiagnosticAccountInfosTraits = DiagnosticApiTraits<AccountInfosTraits>;
	using DiagnosticConfirmTimestampedHashesTraits = DiagnosticApiTraits<ConfirmTimestampedHashesTraits>;
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteDiagnosticApi, DiagnosticAccountInfos)
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_VALID(RemoteDiagnosticApi, DiagnosticConfirmTimestampedHashes)

	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteDiagnosticApi, DiagnosticCounters)
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteDiagnosticApi, ActiveNodeInfos)

	using DiagnosticNamespaceInfosTraits = DiagnosticApiTraits<NamespaceInfosTraits>;
	using DiagnosticMosaicInfosTraits = DiagnosticApiTraits<MosaicInfosTraits>;
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteDiagnosticApi, DiagnosticNamespaceInfos)
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteDiagnosticApi, DiagnosticMosaicInfos)
}}
