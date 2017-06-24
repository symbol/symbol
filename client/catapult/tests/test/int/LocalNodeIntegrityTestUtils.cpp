#include "LocalNodeIntegrityTestUtils.h"
#include "plugins/txes/namespace/src/plugins/MosaicDefinitionTransactionPlugins.h"
#include "plugins/txes/namespace/src/plugins/MosaicSupplyChangeTransactionPlugins.h"
#include "plugins/txes/namespace/src/plugins/RegisterNamespaceTransactionPlugins.h"
#include "plugins/txes/transfer/src/plugins/TransferTransactionPlugins.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/EntityFactory.h"

namespace catapult { namespace test {

	// region ExternalSourceConnection

	std::shared_ptr<model::TransactionRegistry> ExternalSourceConnection::CreateTransactionRegistry() {
		auto pRegistry = std::make_shared<model::TransactionRegistry>();
		pRegistry->registerPlugin(plugins::CreateTransferTransactionPlugin());
		pRegistry->registerPlugin(plugins::CreateRegisterNamespaceTransactionPlugin({ Key(), Address(), Amount(), Amount(), Key() }));
		pRegistry->registerPlugin(plugins::CreateMosaicDefinitionTransactionPlugin({ Key(), Address(), Amount(), Key() }));
		pRegistry->registerPlugin(plugins::CreateMosaicSupplyChangeTransactionPlugin());
		return pRegistry;
	}

	// endregion

	namespace {
		crypto::KeyPair GetNemesisAccountKeyPair() {
			return crypto::KeyPair::FromString(Mijin_Test_Private_Keys[0]);// use a nemesis account
		}

		std::shared_ptr<model::Block> CreateBlock() {
			constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;
			auto signer = GetNemesisAccountKeyPair();

			mocks::MemoryBasedStorage storage;
			auto pNemesisBlockElement = storage.loadBlockElement(Height(1));

			model::PreviousBlockContext context(*pNemesisBlockElement);
			auto pBlock = model::CreateBlock(context, Network_Identifier, signer.publicKey(), model::Transactions());
			pBlock->Timestamp = context.Timestamp + Timestamp(60000);
			SignBlock(signer, *pBlock);
			return std::move(pBlock);
		}
	}

	std::shared_ptr<ionet::PacketIo> PushValidBlock(ExternalSourceConnection& connection) {
		auto pBlock = CreateBlock();
		return PushEntity(connection, ionet::PacketType::Push_Block, pBlock);
	}

	std::shared_ptr<ionet::PacketIo> PushValidTransaction(ExternalSourceConnection& connection) {
		auto pTransaction = CreateTransferTransaction(GetNemesisAccountKeyPair(), GenerateRandomAddress(), Amount(10000));
		return PushEntity(connection, ionet::PacketType::Push_Transactions, std::shared_ptr<model::Transaction>(std::move(pTransaction)));
	}
}}
