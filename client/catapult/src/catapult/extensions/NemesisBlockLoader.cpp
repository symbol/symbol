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
#include "NemesisFundingObserver.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/chain/BlockExecutor.h"
#include "catapult/config/CatapultConfiguration.h"
#include "catapult/crypto/Signer.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/observers/NotificationObserverAdapter.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/utils/IntegerMath.h"

namespace catapult { namespace extensions {

	namespace {
		std::unique_ptr<const observers::NotificationObserver> PrependFundingObserver(
				const Key& nemesisPublicKey,
				NemesisFundingState& nemesisFundingState,
				std::unique_ptr<const observers::NotificationObserver>&& pObserver) {
			observers::DemuxObserverBuilder builder;
			builder.add(CreateNemesisFundingObserver(nemesisPublicKey, nemesisFundingState));
			builder.add(std::move(pObserver));
			return builder.build();
		}

		void LogNemesisBlockInfo(const model::BlockElement& blockElement) {
			auto networkId = blockElement.Block.Network();
			const auto& publicKey = blockElement.Block.SignerPublicKey;
			const auto& generationHash = blockElement.GenerationHash;
			CATAPULT_LOG(info)
					<< std::endl
					<< "      nemesis network id: " << networkId << std::endl
					<< "      nemesis public key: " << publicKey << std::endl
					<< " nemesis generation hash: " << generationHash;
		}

		void OutputNemesisBalance(std::ostream& out, MosaicId mosaicId, Amount amount, char special = ' ') {
			out << std::endl << "      " << special << ' ' << utils::HexFormat(mosaicId) << ": " << amount;
		}

		void LogNemesisBalances(MosaicId currencyMosaicId, MosaicId harvestingMosaicId, const state::AccountBalances& balances) {
			std::ostringstream out;
			out << "Nemesis Mosaics:";
			OutputNemesisBalance(out, currencyMosaicId, balances.get(currencyMosaicId), 'C');
			OutputNemesisBalance(out, harvestingMosaicId, balances.get(harvestingMosaicId), 'H');

			for (const auto& pair : balances) {
				if (currencyMosaicId != pair.first && harvestingMosaicId != pair.first)
					OutputNemesisBalance(out, pair.first, pair.second);
			}

			CATAPULT_LOG(info) << out.str();
		}

		void CheckNemesisBlockInfo(const model::BlockElement& blockElement, const model::NetworkInfo& expectedNetwork) {
			auto networkId = blockElement.Block.Network();
			const auto& publicKey = blockElement.Block.SignerPublicKey;
			const auto& generationHash = blockElement.GenerationHash;

			if (expectedNetwork.Identifier != networkId)
				CATAPULT_THROW_INVALID_ARGUMENT_1("nemesis network id does not match network", networkId);

			if (expectedNetwork.PublicKey != publicKey)
				CATAPULT_THROW_INVALID_ARGUMENT_1("nemesis public key does not match network", publicKey);

			if (expectedNetwork.GenerationHash != generationHash)
				CATAPULT_THROW_INVALID_ARGUMENT_1("nemesis generation hash does not match network", generationHash);
		}

		void CheckNemesisBlockTransactionTypes(const model::Block& block, const model::TransactionRegistry& transactionRegistry) {
			for (const auto& transaction : block.Transactions()) {
				const auto* pPlugin = transactionRegistry.findPlugin(transaction.Type);
				if (!pPlugin || !pPlugin->supportsTopLevel())
					CATAPULT_THROW_INVALID_ARGUMENT_1("nemesis block contains unsupported transaction type", transaction.Type);
			}
		}

		void CheckNemesisBlockFeeMultiplier(const model::Block& block) {
			if (BlockFeeMultiplier() != block.FeeMultiplier)
				CATAPULT_THROW_INVALID_ARGUMENT_1("nemesis block contains non-zero fee multiplier", block.FeeMultiplier);
		}

		void CheckImportanceAndBalanceConsistency(Importance totalChainImportance, Amount totalChainBalance) {
			if (!utils::IsPowerMultiple<uint64_t>(totalChainImportance.unwrap(), totalChainBalance.unwrap(), 10)) {
				std::ostringstream out;
				out
						<< "harvesting outflows (" << totalChainBalance << ") do not add up to power ten multiple of "
						<< "expected importance (" << totalChainImportance << ")";
				CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
			}
		}

		void CheckInitialCurrencyAtomicUnits(Amount expectedInitialCurrencyAtomicUnits, Amount initialCurrencyAtomicUnits) {
			if (expectedInitialCurrencyAtomicUnits != initialCurrencyAtomicUnits) {
				std::ostringstream out;
				out
						<< "currency outflows (" << initialCurrencyAtomicUnits << ") do not equal the "
						<< "expected initial currency atomic units (" << expectedInitialCurrencyAtomicUnits << ")";
				CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
			}
		}

		void CheckMaxMosaicAtomicUnits(const state::AccountBalances& totalFundedMosaics, Amount maxMosaicAtomicUnits) {
			for (const auto& pair : totalFundedMosaics) {
				if (maxMosaicAtomicUnits < pair.second) {
					std::ostringstream out;
					out
							<< "currency outflows (" << pair.second << ") for mosaic id " << pair.first
							<< " exceed max allowed atomic units (" << maxMosaicAtomicUnits << ")";
					CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
				}
			}
		}
	}

	NemesisBlockLoader::NemesisBlockLoader(
			cache::CatapultCacheDelta& cacheDelta,
			const plugins::PluginManager& pluginManager,
			std::unique_ptr<const observers::NotificationObserver>&& pObserver)
			: m_cacheDelta(cacheDelta)
			, m_pluginManager(pluginManager)
			, m_pObserver(std::make_unique<observers::NotificationObserverAdapter>(
					PrependFundingObserver(m_nemesisPublicKey, m_nemesisFundingState, std::move(pObserver)),
					pluginManager.createNotificationPublisher()))
			, m_pPublisher(pluginManager.createNotificationPublisher())
	{}

	void NemesisBlockLoader::execute(const LocalNodeStateRef& stateRef, StateHashVerification stateHashVerification) {
		// 1. load the nemesis block
		auto storageView = stateRef.Storage.view();
		auto pNemesisBlockElement = storageView.loadBlockElement(Height(1));

		// 2. execute the nemesis block
		execute(stateRef.Config.BlockChain, *pNemesisBlockElement, stateHashVerification, Verbosity::On);
	}

	void NemesisBlockLoader::executeAndCommit(const LocalNodeStateRef& stateRef, StateHashVerification stateHashVerification) {
		// 1. execute the nemesis block
		execute(stateRef, stateHashVerification);

		// 2. commit changes
		stateRef.Cache.commit(Height(1));
	}

	void NemesisBlockLoader::execute(const model::BlockChainConfiguration& config, const model::BlockElement& nemesisBlockElement) {
		execute(config, nemesisBlockElement, StateHashVerification::Enabled, Verbosity::Off);
	}

	namespace {
		void RequireHashMatch(const char* hashDescription, const Hash256& blockHash, const Hash256& calculatedHash) {
			if (blockHash == calculatedHash)
				return;

			std::ostringstream out;
			out
					<< "nemesis block " << hashDescription << " hash (" << blockHash << ") does not match "
					<< "calculated " << hashDescription << " hash (" << calculatedHash << ")";
			CATAPULT_THROW_RUNTIME_ERROR(out.str().c_str());
		}

		void RequireValidSignature(const model::Block& block) {
			auto headerSize = model::VerifiableEntity::Header_Size;
			auto blockData = RawBuffer{ reinterpret_cast<const uint8_t*>(&block) + headerSize, sizeof(model::BlockHeader) - headerSize };
			if (crypto::Verify(block.SignerPublicKey, blockData, block.Signature))
				return;

			CATAPULT_THROW_RUNTIME_ERROR("nemesis block has invalid signature");
		}
	}

	void NemesisBlockLoader::execute(
			const model::BlockChainConfiguration& config,
			const model::BlockElement& nemesisBlockElement,
			StateHashVerification stateHashVerification,
			Verbosity verbosity) {
		// 1. check the nemesis block
		if (Verbosity::On == verbosity)
			LogNemesisBlockInfo(nemesisBlockElement);

		CheckNemesisBlockInfo(nemesisBlockElement, config.Network);
		CheckNemesisBlockTransactionTypes(nemesisBlockElement.Block, m_pluginManager.transactionRegistry());
		CheckNemesisBlockFeeMultiplier(nemesisBlockElement.Block);

		// 2. reset nemesis funding observer data
		m_nemesisPublicKey = nemesisBlockElement.Block.SignerPublicKey;
		m_nemesisFundingState = NemesisFundingState();

		// 3. execute the block
		auto readOnlyCache = m_cacheDelta.toReadOnly();
		auto resolverContext = m_pluginManager.createResolverContext(readOnlyCache);
		auto blockStatementBuilder = model::BlockStatementBuilder();
		auto observerState = config.EnableVerifiableReceipts
				? observers::ObserverState(m_cacheDelta, blockStatementBuilder)
				: observers::ObserverState(m_cacheDelta);
		chain::ExecuteBlock(nemesisBlockElement, { *m_pObserver, resolverContext, observerState });

		// 4. check the funded balances are reasonable
		if (Verbosity::On == verbosity)
			LogNemesisBalances(config.CurrencyMosaicId, config.HarvestingMosaicId, m_nemesisFundingState.TotalFundedMosaics);

		CheckImportanceAndBalanceConsistency(
				config.TotalChainImportance,
				m_nemesisFundingState.TotalFundedMosaics.get(config.HarvestingMosaicId));

		CheckInitialCurrencyAtomicUnits(
				config.InitialCurrencyAtomicUnits,
				m_nemesisFundingState.TotalFundedMosaics.get(config.CurrencyMosaicId));

		CheckMaxMosaicAtomicUnits(m_nemesisFundingState.TotalFundedMosaics, config.MaxMosaicAtomicUnits);

		// 5. check the hashes
		auto calculatedReceiptsHash = model::CalculateMerkleHash(*blockStatementBuilder.build());
		RequireHashMatch("receipts", nemesisBlockElement.Block.ReceiptsHash, calculatedReceiptsHash);

		// important: do not remove calculateStateHash call because it has important side-effect of populating the patricia tree delta
		auto cacheStateHash = m_cacheDelta.calculateStateHash(Height(1)).StateHash;
		if (StateHashVerification::Enabled == stateHashVerification)
			RequireHashMatch("state", nemesisBlockElement.Block.StateHash, cacheStateHash);

		// 6. check the block signature
		RequireValidSignature(nemesisBlockElement.Block);
	}
}}
