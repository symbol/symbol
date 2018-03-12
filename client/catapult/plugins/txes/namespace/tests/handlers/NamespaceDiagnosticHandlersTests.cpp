#include "src/handlers/NamespaceDiagnosticHandlers.h"
#include "src/model/MosaicInfo.h"
#include "src/model/NamespaceInfo.h"
#include "tests/test/plugins/BatchHandlerTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

	namespace {

		// region mosaic infos

		using MosaicInfos = std::vector<std::shared_ptr<const model::MosaicInfo>>;

		struct MosaicInfosTraits {
		public:
			using RequestStructureType = MosaicId;
			using ResponseType = MosaicInfos;
			static constexpr auto Packet_Type = ionet::PacketType::Mosaic_Infos;
			static constexpr auto Valid_Request_Payload_Size = sizeof(MosaicId);
			static constexpr auto Register_Handler_Func = RegisterMosaicInfosHandler;
			static constexpr auto Message() { return "mosaic at "; }

		public:
			struct ResponseState {};

		public:
			static ResponseType CreateResponse(size_t count, const ResponseState&) {
				ResponseType mosaicInfos;
				for (auto i = 0u; i < count; ++i) {
					auto pInfo = std::make_shared<model::MosaicInfo>();
					pInfo->Id = MosaicId(123 + i);
					mosaicInfos.push_back(pInfo);
				}

				return mosaicInfos;
			}

			static size_t TotalSize(const ResponseType& result) {
				return result.size() * sizeof(model::MosaicInfo);
			}

			static void AssertExpectedResponse(const ionet::PacketPayload& payload, const ResponseType& expectedResult) {
				ASSERT_EQ(1u, payload.buffers().size());

				auto i = 0u;
				const auto* pMosaicInfo = reinterpret_cast<const model::MosaicInfo*>(payload.buffers()[0].pData);
				for (const auto& pExpectedMosaicInfo : expectedResult) {
					EXPECT_EQ(pExpectedMosaicInfo->Id, pMosaicInfo->Id) << Message() << i;
					++pMosaicInfo;
					++i;
				}
			}
		};

		// endregion

		// region namespace infos

		using NamespaceInfos = std::vector<std::shared_ptr<const model::NamespaceInfo>>;

		struct NamespaceInfosTraits {
		public:
			using RequestStructureType = NamespaceId;
			using ResponseType = NamespaceInfos;
			static constexpr auto Packet_Type = ionet::PacketType::Namespace_Infos;
			static constexpr auto Valid_Request_Payload_Size = sizeof(NamespaceId);
			static constexpr auto Register_Handler_Func = RegisterNamespaceInfosHandler;
			static constexpr auto Message() { return "namespace at "; }

		public:
			struct ResponseState {};

		public:
			static ResponseType CreateResponse(size_t count, const ResponseState&) {
				ResponseType namespaceInfos;
				for (auto i = 0u; i < count; ++i) {
					auto pInfo = std::make_shared<model::NamespaceInfo>();
					pInfo->Id = NamespaceId(123 + i);
					namespaceInfos.push_back(pInfo);
				}

				return namespaceInfos;
			}

			static size_t TotalSize(const ResponseType& result) {
				return test::TotalSize(result);
			}

			static void AssertExpectedResponse(const ionet::PacketPayload& payload, const ResponseType& expectedResult) {
				ASSERT_EQ(expectedResult.size(), payload.buffers().size());

				auto i = 0u;
				for (const auto& pExpectedNamespaceInfo : expectedResult) {
					auto pNamespaceInfo = reinterpret_cast<const model::NamespaceInfo*>(payload.buffers()[i].pData);
					EXPECT_EQ(pExpectedNamespaceInfo->Id, pNamespaceInfo->Id) << Message() << i;
					++i;
				}
			}
		};

		// endregion
	}

	DEFINE_BATCH_HANDLER_TESTS(NamespaceDiagnosticHandlersTests, MosaicInfos)
	DEFINE_BATCH_HANDLER_TESTS(NamespaceDiagnosticHandlersTests, NamespaceInfos)
}}
