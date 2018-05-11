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

#include "LocalNodeIntegrityTestUtils.h"
#include "plugins/txes/namespace/src/plugins/MosaicDefinitionTransactionPlugin.h"
#include "plugins/txes/namespace/src/plugins/MosaicSupplyChangeTransactionPlugin.h"
#include "plugins/txes/namespace/src/plugins/RegisterNamespaceTransactionPlugin.h"
#include "plugins/txes/transfer/src/plugins/TransferTransactionPlugin.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/EntityFactory.h"

namespace catapult { namespace test {

	// region ExternalSourceConnection

	model::TransactionRegistry ExternalSourceConnection::CreateTransactionRegistry() {
		auto registry = model::TransactionRegistry();
		registry.registerPlugin(plugins::CreateTransferTransactionPlugin());
		registry.registerPlugin(plugins::CreateRegisterNamespaceTransactionPlugin({ Key(), Address(), Amount(), Amount(), Key() }));
		registry.registerPlugin(plugins::CreateMosaicDefinitionTransactionPlugin({ Key(), Address(), Amount(), Key() }));
		registry.registerPlugin(plugins::CreateMosaicSupplyChangeTransactionPlugin());
		return registry;
	}

	// endregion

	namespace {
		crypto::KeyPair GetNemesisAccountKeyPair() {
			return crypto::KeyPair::FromString(Mijin_Test_Private_Keys[0]);// use a nemesis account
		}

		std::shared_ptr<model::Block> CreateBlock() {
			constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;
			auto signer = GetNemesisAccountKeyPair();

			mocks::MockMemoryBasedStorage storage;
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
