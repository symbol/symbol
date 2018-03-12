#include "src/handlers/CoreDiagnosticHandlers.h"
#include "catapult/state/AccountStateAdapter.h"
#include "tests/test/plugins/BatchHandlerTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

	namespace {
		using AccountInfos = std::vector<std::shared_ptr<const model::AccountInfo>>;

		struct AccountInfosTraits {
		public:
			using RequestStructureType = Address;
			using ResponseType = AccountInfos;
			static constexpr auto Packet_Type = ionet::PacketType::Account_Infos;
			static constexpr auto Valid_Request_Payload_Size = Address_Decoded_Size;
			static constexpr auto Register_Handler_Func = RegisterAccountInfosHandler;
			static constexpr auto Message() { return "address at "; }

		public:
			struct ResponseState {};

		public:
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
