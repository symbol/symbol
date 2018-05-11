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

#include "src/handlers/CoreDiagnosticHandlers.h"
#include "catapult/state/AccountStateAdapter.h"
#include "tests/test/plugins/BatchHandlerTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

	namespace {
		struct AccountInfosTraits {
		public:
			using RequestStructureType = Address;
			using ResponseType = std::vector<std::shared_ptr<const model::AccountInfo>>;
			static constexpr auto Packet_Type = ionet::PacketType::Account_Infos;
			static constexpr auto Valid_Request_Payload_Size = Address_Decoded_Size;
			static constexpr auto Message() { return "address at "; }

		public:
			struct ResponseState {};

		public:
			template<typename TAction>
			static void RegisterHandler(ionet::ServerPacketHandlers& handlers, TAction action) {
				RegisterAccountInfosHandler(handlers, test::BatchHandlerSupplierActionToProducer<ResponseType>(action));
			}

			static ResponseType CreateResponse(size_t count, const ResponseState&) {
				ResponseType accountInfos;
				for (auto i = 0u; i < count; ++i) {
					state::AccountState accountState(test::GenerateRandomData<Address_Decoded_Size>(), Height(123 + i));
					accountInfos.push_back(state::ToAccountInfo(accountState));
				}

				return accountInfos;
			}

			static size_t TotalSize(const ResponseType& result) {
				return test::TotalSize(result);
			}

			static void AssertExpectedResponse(const ionet::PacketPayload& payload, const ResponseType& expectedResult) {
				ASSERT_EQ(expectedResult.size(), payload.buffers().size());

				auto i = 0u;
				for (const auto& pExpectedAccountInfo : expectedResult) {
					auto pAccountInfo = reinterpret_cast<const model::AccountInfo*>(payload.buffers()[i].pData);
					EXPECT_EQ(pExpectedAccountInfo->Address, pAccountInfo->Address) << Message() << i;
					EXPECT_EQ(pExpectedAccountInfo->AddressHeight, pAccountInfo->AddressHeight) << Message() << i;
					++i;
				}
			}
		};
	}

	DEFINE_BATCH_HANDLER_TESTS(CoreDiagnosticHandlersTests, AccountInfos)
}}
