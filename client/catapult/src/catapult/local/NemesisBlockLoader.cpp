#include "NemesisBlockLoader.h"
#include "LocalNodeStateRef.h"
#include "NemesisBalanceTransferSubscriber.h"
#include "catapult/cache/AccountStateCache.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/chain/BlockExecutor.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/TransactionPlugin.h"

namespace catapult { namespace local {

	namespace {
		using BalanceTransfers = NemesisBalanceTransferSubscriber::BalanceTransfers;

		void LogNemesisBlockInfo(const model::BlockElement& blockElement, const model::NetworkInfo& expectedNetwork) {
			auto networkId = static_cast<model::NetworkIdentifier>(blockElement.Block.Network());
			const auto& publicKey = blockElement.Block.Signer;
			const auto& generationHash = blockElement.GenerationHash;
			CATAPULT_LOG(info)
					<< std::endl
					<< "     nemesis network id: " << networkId << std::endl
					<< "     nemesis public key: " << utils::HexFormat(publicKey) << std::endl
					<< "nemesis generation hash: " << utils::HexFormat(generationHash);

			if (expectedNetwork.Identifier != networkId)
				CATAPULT_THROW_INVALID_ARGUMENT_1("nemesis network id does not match network", networkId);

			if (expectedNetwork.PublicKey != publicKey)
				CATAPULT_THROW_INVALID_ARGUMENT_1("nemesis public key does not match network", utils::HexFormat(publicKey));

			if (expectedNetwork.GenerationHash != generationHash)
				CATAPULT_THROW_INVALID_ARGUMENT_1("nemesis generation hash does not match network", utils::HexFormat(generationHash));
		}

		BalanceTransfers GetBlockOutflows(
				const model::Block& block,
				const model::TransactionRegistry& registry,
				const model::NotificationPublisher& publisher) {
			NemesisBalanceTransferSubscriber subscriber(block.Signer);

			Hash256 placeholderHash{};
			for (const auto& transaction : block.Transactions()) {
				if (!registry.findPlugin(transaction.Type))
					CATAPULT_THROW_INVALID_ARGUMENT_1("nemesis block contains unsupported transaction type", transaction.Type);

				publisher.publish(model::WeakEntityInfo(transaction, placeholderHash), subscriber);
			}

			return subscriber.outflows();
		}

		void LogOutflows(const BalanceTransfers& transfers, Amount expectedXemBalance) {
			for (const auto& transfer : transfers)
				CATAPULT_LOG(info) << "nemesis block seeded with " << utils::HexFormat(transfer.first) << ": " << transfer.second;

			auto iter = transfers.find(Xem_Id);
			if (transfers.cend() == iter || expectedXemBalance != iter->second)
				CATAPULT_THROW_INVALID_ARGUMENT_1("xem outflows do not add up to expected", expectedXemBalance);
		}

		void PrepareNemesisAccount(const Key& nemesisPublicKey, const BalanceTransfers& transfers, cache::CatapultCacheDelta& cacheDelta) {
			auto& accountStateCache = cacheDelta.sub<cache::AccountStateCache>();
			auto pNemesisState = accountStateCache.addAccount(nemesisPublicKey, Height(1));
			for (const auto& transfer : transfers)
				pNemesisState->Balances.credit(transfer.first, transfer.second);
		}
	}

	void LoadNemesisBlock(
			const model::TransactionRegistry& registry,
			const observers::EntityObserver& observer,
			const LocalNodeStateRef& stateRef,
			const PreCommitFunc& preCommit) {
		// - load the nemesis block
		auto storageView = stateRef.Storage.view();
		auto pNemesisBlockElement = storageView.loadBlockElement(Height(1));
		LogNemesisBlockInfo(*pNemesisBlockElement, stateRef.Config.BlockChain.Network);

		// - visit the nemesis block
		const auto& nemesisBlock = pNemesisBlockElement->Block;
		auto pPublisher = CreateNotificationPublisher(registry);
		auto outflows = GetBlockOutflows(nemesisBlock, registry, *pPublisher);
		LogOutflows(outflows, stateRef.Config.BlockChain.TotalChainBalance);

		// - prepare the nemesis account
		auto cacheDelta = stateRef.Cache.createDelta();
		PrepareNemesisAccount(nemesisBlock.Signer, outflows, cacheDelta);

		// - execute the block
		auto observerState = observers::ObserverState(cacheDelta, stateRef.State);
		chain::ExecuteBlock(*pNemesisBlockElement, observer, observerState);

		// - give the caller a chance to act before commiting
		if (preCommit)
			preCommit(cacheDelta);

		// - commit changes
		stateRef.Cache.commit(nemesisBlock.Height);
	}
}}
