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

#include "NemesisBlockLoader.h"
#include "LocalNodeStateRef.h"
#include "NemesisBalanceTransferSubscriber.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/chain/BlockExecutor.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/TransactionPlugin.h"

namespace catapult { namespace extensions {

	namespace {
		using BalanceTransfers = NemesisBalanceTransferSubscriber::BalanceTransfers;

		void LogNemesisBlockInfo(const model::BlockElement& blockElement) {
			auto networkId = blockElement.Block.Network();
			const auto& publicKey = blockElement.Block.Signer;
			const auto& generationHash = blockElement.GenerationHash;
			CATAPULT_LOG(info)
					<< std::endl
					<< "     nemesis network id: " << networkId << std::endl
					<< "     nemesis public key: " << utils::HexFormat(publicKey) << std::endl
					<< "nemesis generation hash: " << utils::HexFormat(generationHash);
		}

		void CheckNemesisBlockInfo(const model::BlockElement& blockElement, const model::NetworkInfo& expectedNetwork) {
			auto networkId = blockElement.Block.Network();
			const auto& publicKey = blockElement.Block.Signer;
			const auto& generationHash = blockElement.GenerationHash;

			if (expectedNetwork.Identifier != networkId)
				CATAPULT_THROW_INVALID_ARGUMENT_1("nemesis network id does not match network", networkId);

			if (expectedNetwork.PublicKey != publicKey)
				CATAPULT_THROW_INVALID_ARGUMENT_1("nemesis public key does not match network", utils::HexFormat(publicKey));

			if (expectedNetwork.GenerationHash != generationHash)
				CATAPULT_THROW_INVALID_ARGUMENT_1("nemesis generation hash does not match network", utils::HexFormat(generationHash));
		}

		BalanceTransfers GetBlockOutflows(
				const model::Block& block,
				const model::TransactionRegistry& transactionRegistry,
				const model::NotificationPublisher& publisher) {
			NemesisBalanceTransferSubscriber subscriber(block.Signer);

			Hash256 placeholderHash{};
			for (const auto& transaction : block.Transactions()) {
				if (!transactionRegistry.findPlugin(transaction.Type))
					CATAPULT_THROW_INVALID_ARGUMENT_1("nemesis block contains unsupported transaction type", transaction.Type);

				publisher.publish(model::WeakEntityInfo(transaction, placeholderHash), subscriber);
			}

			return subscriber.outflows();
		}

		void LogOutflows(const BalanceTransfers& transfers) {
			for (const auto& transfer : transfers)
				CATAPULT_LOG(info) << "nemesis block seeded with " << utils::HexFormat(transfer.first) << ": " << transfer.second;
		}

		void CheckOutflows(const BalanceTransfers& transfers, Amount expectedXemBalance) {
			auto iter = transfers.find(Xem_Id);
			if (transfers.cend() == iter || expectedXemBalance != iter->second)
				CATAPULT_THROW_INVALID_ARGUMENT_1("xem outflows do not add up to expected", expectedXemBalance);
		}

		void PrepareNemesisAccount(const Key& nemesisPublicKey, const BalanceTransfers& transfers, cache::CatapultCacheDelta& cacheDelta) {
			auto& accountStateCache = cacheDelta.sub<cache::AccountStateCache>();
			auto& nemesisState = accountStateCache.addAccount(nemesisPublicKey, Height(1));
			for (const auto& transfer : transfers)
				nemesisState.Balances.credit(transfer.first, transfer.second);
		}
	}

	NemesisBlockLoader::NemesisBlockLoader(
			const model::TransactionRegistry& transactionRegistry,
			const model::NotificationPublisher& publisher,
			const observers::EntityObserver& observer)
			: m_transactionRegistry(transactionRegistry)
			, m_publisher(publisher)
			, m_observer(observer)
	{}

	void NemesisBlockLoader::executeAndCommit(const LocalNodeStateRef& stateRef) const {
		// 1. load the nemesis block
		auto storageView = stateRef.Storage.view();
		auto pNemesisBlockElement = storageView.loadBlockElement(Height(1));

		// 2. execute the nemesis block
		auto cacheDelta = stateRef.Cache.createDelta();
		execute(stateRef.Config.BlockChain, *pNemesisBlockElement, cacheDelta, stateRef.State, Verbosity::On);

		// 3. commit changes
		stateRef.Cache.commit(pNemesisBlockElement->Block.Height);
	}

	void NemesisBlockLoader::execute(
			const model::BlockChainConfiguration& config,
			const model::BlockElement& nemesisBlockElement,
			cache::CatapultCacheDelta& cacheDelta) const {
		auto catapultState = state::CatapultState();
		execute(config, nemesisBlockElement, cacheDelta, catapultState, Verbosity::Off);
	}

	void NemesisBlockLoader::execute(
			const model::BlockChainConfiguration& config,
			const model::BlockElement& nemesisBlockElement,
			cache::CatapultCacheDelta& cacheDelta,
			state::CatapultState& catapultState,
			Verbosity verbosity) const {
		// 1. check the nemesis block
		if (Verbosity::On == verbosity)
			LogNemesisBlockInfo(nemesisBlockElement);

		CheckNemesisBlockInfo(nemesisBlockElement, config.Network);

		// 2. collect nemesis outflows
		const auto& nemesisBlock = nemesisBlockElement.Block;
		auto outflows = GetBlockOutflows(nemesisBlock, m_transactionRegistry, m_publisher);
		if (Verbosity::On == verbosity)
			LogOutflows(outflows);

		CheckOutflows(outflows, config.TotalChainBalance.microxem());

		// 3. prepare the nemesis account
		PrepareNemesisAccount(nemesisBlock.Signer, outflows, cacheDelta);

		// 4. execute the block
		auto observerState = observers::ObserverState(cacheDelta, catapultState);
		chain::ExecuteBlock(nemesisBlockElement, m_observer, observerState);
	}
}}
