#include "src/handlers/HashCacheDiagnosticHandlers.h"
#include "tests/test/plugins/BatchHandlerTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

	namespace {
		using UnconfirmedTimestampedHashes = std::vector<const state::TimestampedHash*>;

		struct ConfirmTimestampedHashesTraits {
		public:
			using RequestEntityType = state::TimestampedHash;
			using ResponseType = UnconfirmedTimestampedHashes;
			static constexpr auto Packet_Type = ionet::PacketType::Confirm_Timestamped_Hashes;
			static constexpr auto Valid_Payload_Size = sizeof(state::TimestampedHash);
			static constexpr auto Register_Handler_Func = RegisterConfirmTimestampedHashesHandler;
			static constexpr auto Message() { return "timestamped hash at "; }

		public:
			struct ResponseState {
				std::vector<state::TimestampedHash> TimestampedHashes;
			};

		public:
			static ResponseType CreateResponse(size_t count, ResponseState& state) {
				ResponseType response;
				state.TimestampedHashes.reserve(count);
				for (auto i = 0u; i < count; ++i) {
					state::TimestampedHash timestampedHash(Timestamp(123 + i), test::GenerateRandomData<Hash256_Size>());
					state.TimestampedHashes.push_back(timestampedHash);
					response.push_back(&state.TimestampedHashes.back());
				}

				return response;
			}

			static size_t TotalSize(const ResponseType& result) {
				return result.size() * sizeof(state::TimestampedHash);
			}

			static void AssertExpectedResponse(const ionet::PacketPayload& payload, const ResponseType& expectedResult) {
				ASSERT_EQ(1u, payload.buffers().size());

				auto i = 0u;
				auto pTimestampedHash = reinterpret_cast<const state::TimestampedHash*>(payload.buffers()[0].pData);
				for (const auto& pExpectedTimestampedHash : expectedResult) {
					EXPECT_EQ(*pExpectedTimestampedHash, *pTimestampedHash++) << Message() << i;
					++i;
				}
			}
		};
	}

	DEFINE_BATCH_HANDLER_TESTS(HashCacheDiagnosticHandlersTests, ConfirmTimestampedHashes)
}}
