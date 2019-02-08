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

#include "BlockExecutionHashesCalculator.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/chain/ExecutionConfiguration.h"
#include "catapult/chain/ProcessingNotificationSubscriber.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/Elements.h"

namespace catapult { namespace harvesting {

	namespace {
		model::BlockElement ExplodeToBlockElement(const model::Block& block, const std::vector<Hash256>& transactionHashes) {
			// block hash is not used, so zero it out
			model::BlockElement blockElement(block);
			blockElement.EntityHash = {};

			auto counter = 0u;
			for (const auto& transaction : block.Transactions()) {
				if (counter >= transactionHashes.size())
					CATAPULT_THROW_INVALID_ARGUMENT_1("too few transaction hashes were provided", transactionHashes.size());

				blockElement.Transactions.push_back(model::TransactionElement(transaction));

				auto& entityHash = blockElement.Transactions.back().EntityHash;
				entityHash = transactionHashes[counter++];
			}

			if (counter != transactionHashes.size())
				CATAPULT_THROW_INVALID_ARGUMENT_1("too many transaction hashes were provided", transactionHashes.size());

			return blockElement;
		}
	}

	BlockExecutionHashes CalculateBlockExecutionHashes(
			const model::Block& block,
			const std::vector<Hash256>& transactionHashes,
			const cache::CatapultCache& cache,
			const model::BlockChainConfiguration& config,
			const chain::ExecutionConfiguration& executionConfig) {
		// 0. bypass calculation if disabled
		if (!config.ShouldEnableVerifiableState && !config.ShouldEnableVerifiableReceipts)
			return BlockExecutionHashes(true);

		// 1. prepare observer state (for the *next* harvested block)
		const auto& accountStateCache = cache.sub<cache::AccountStateCache>();
		auto importanceHeight = model::ConvertToImportanceHeight(block.Height, accountStateCache.importanceGrouping());

		auto cacheDetachedDelta = cache.createDetachableDelta().detach();
		auto pCacheDelta = cacheDetachedDelta.lock();
		auto catapultState = state::CatapultState();
		catapultState.LastRecalculationHeight = importanceHeight;
		auto blockStatementBuilder = model::BlockStatementBuilder();
		auto observerState = config.ShouldEnableVerifiableReceipts
				? observers::ObserverState(*pCacheDelta, catapultState, blockStatementBuilder)
				: observers::ObserverState(*pCacheDelta, catapultState);

		// 2. prepare subscriber
		auto network = executionConfig.Network;
		auto readOnlyCache = pCacheDelta->toReadOnly();
		auto resolverContext = executionConfig.ResolverContextFactory(readOnlyCache);
		auto validatorContext = validators::ValidatorContext(block.Height, block.Timestamp, network, resolverContext, readOnlyCache);
		auto observerContext = observers::ObserverContext(observerState, block.Height, observers::NotifyMode::Commit, resolverContext);

		const auto& validator = *executionConfig.pValidator;
		const auto& observer = *executionConfig.pObserver;
		chain::ProcessingNotificationSubscriber sub(validator, validatorContext, observer, observerContext);

		// 3. execute block
		model::WeakEntityInfos entityInfos;
		auto blockElement = ExplodeToBlockElement(block, transactionHashes);
		model::ExtractEntityInfos(blockElement, entityInfos);

		for (const auto& entityInfo : entityInfos) {
			executionConfig.pNotificationPublisher->publish(entityInfo, sub);

			auto executionResult = sub.result();
			if (!IsValidationResultSuccess(executionResult)) {
				CATAPULT_LOG(debug)
						<< "bypassing block containing entity " << utils::HexFormat(entityInfo.hash())
						<< " due to execution failure " << executionResult;
				return BlockExecutionHashes(false);
			}
		}

		// 4. extract results
		BlockExecutionHashes blockExecutionHashes(true);
		if (config.ShouldEnableVerifiableState)
			blockExecutionHashes.StateHash = pCacheDelta->calculateStateHash(block.Height).StateHash;

		if (config.ShouldEnableVerifiableReceipts)
			blockExecutionHashes.ReceiptsHash = model::CalculateMerkleHash(*blockStatementBuilder.build());

		return blockExecutionHashes;
	}
}}
