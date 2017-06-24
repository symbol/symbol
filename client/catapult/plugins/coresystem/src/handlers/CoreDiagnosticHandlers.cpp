#include "CoreDiagnosticHandlers.h"
#include "catapult/handlers/HandlerFactory.h"

namespace catapult { namespace handlers {

	namespace {
		struct AccountInfosTraits {
			using EntityType = Address;
			using SupplierResultsType = std::vector<std::shared_ptr<const model::AccountInfo>>;

			static constexpr auto Packet_Type = ionet::PacketType::Account_Infos;

			static auto ToPayload(const SupplierResultsType& results) {
				return ionet::PacketPayload::FromEntities(Packet_Type, results);
			}
		};
	}

	void RegisterAccountInfosHandler(ionet::ServerPacketHandlers& handlers, const AccountInfosSupplier& accountInfosSupplier) {
		using HandlerFactory = BatchHandlerFactory<AccountInfosTraits>;
		handlers.registerHandler(HandlerFactory::Packet_Type, HandlerFactory::Create(accountInfosSupplier));
	}
}}
