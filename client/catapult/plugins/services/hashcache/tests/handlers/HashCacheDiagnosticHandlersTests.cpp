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

#include "src/handlers/HashCacheDiagnosticHandlers.h"
#include "tests/test/plugins/BatchHandlerTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

	namespace {
		using UnconfirmedTimestampedHashes = std::vector<const state::TimestampedHash*>;

		struct ConfirmTimestampedHashesTraits {
		public:
			using RequestStructureType = state::TimestampedHash;
			using ResponseType = UnconfirmedTimestampedHashes;
			static constexpr auto Packet_Type = ionet::PacketType::Confirm_Timestamped_Hashes;
			static constexpr auto Valid_Request_Payload_Size = SizeOf32<state::TimestampedHash>();
			static constexpr auto Message = "timestamped hash at ";

		public:
			struct ResponseState {
				std::vector<state::TimestampedHash> TimestampedHashes;
			};

		public:
			template<typename TAction>
			static void RegisterHandler(ionet::ServerPacketHandlers& handlers, TAction action) {
				RegisterConfirmTimestampedHashesHandler(handlers, test::BatchHandlerSupplierActionToProducer<ResponseType>(action));
			}

			static ResponseType CreateResponse(size_t count, ResponseState& state) {
				ResponseType response;
				state.TimestampedHashes.reserve(count);
				for (auto i = 0u; i < count; ++i) {
					state::TimestampedHash timestampedHash(Timestamp(123 + i), test::GenerateRandomByteArray<Hash256>());
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
					EXPECT_EQ(*pExpectedTimestampedHash, *pTimestampedHash++) << Message << i;
					++i;
				}
			}
		};
	}

	DEFINE_BATCH_HANDLER_TESTS(HashCacheDiagnosticHandlersTests, ConfirmTimestampedHashes)
}}
