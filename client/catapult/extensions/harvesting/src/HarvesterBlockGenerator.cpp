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

#include "HarvesterBlockGenerator.h"
#include "HarvestingUtFacadeFactory.h"
#include "TransactionsInfoSupplier.h"
#include "catapult/model/TransactionPlugin.h"

namespace catapult { namespace harvesting {

	namespace {
		std::unique_ptr<model::Block> GenerateBlock(
				HarvestingUtFacade& facade,
				const model::BlockHeader& originalBlockHeader,
				const TransactionsInfo& transactionsInfo) {
			// copy and update block header
			model::BlockHeader blockHeader;
			std::memcpy(static_cast<void*>(&blockHeader), &originalBlockHeader, sizeof(model::BlockHeader));
			blockHeader.TransactionsHash = transactionsInfo.TransactionsHash;
			blockHeader.FeeMultiplier = transactionsInfo.FeeMultiplier;

			// generate the block
			return facade.commit(blockHeader);
		}
	}

	BlockGenerator CreateHarvesterBlockGenerator(
			model::TransactionSelectionStrategy strategy,
			const model::TransactionRegistry& transactionRegistry,
			const HarvestingUtFacadeFactory& utFacadeFactory,
			const cache::ReadWriteUtCache& utCache) {
		auto countRetriever = [&transactionRegistry](const auto& transaction) {
			return 1 + transactionRegistry.findPlugin(transaction.Type)->embeddedCount(transaction);
		};

		auto transactionsInfoSupplier = CreateTransactionsInfoSupplier(strategy, countRetriever, utCache);
		return [utFacadeFactory, transactionsInfoSupplier](const auto& blockHeader, auto maxTransactionsPerBlock) {
			// 1. check height consistency
			auto pUtFacade = utFacadeFactory.create(blockHeader.Timestamp);
			if (blockHeader.Height != pUtFacade->height()) {
				CATAPULT_LOG(debug)
						<< "bypassing state hash calculation because cache height (" << pUtFacade->height() - Height(1)
						<< ") is inconsistent with block height (" << blockHeader.Height << ")";
				return std::unique_ptr<model::Block>();
			}

			// 2. select transactions
			auto transactionsInfo = transactionsInfoSupplier(*pUtFacade, maxTransactionsPerBlock);

			// 3. build a block
			auto pBlock = GenerateBlock(*pUtFacade, blockHeader, transactionsInfo);
			if (!pBlock) {
				CATAPULT_LOG(warning) << "failed to generate harvested block";
				return std::unique_ptr<model::Block>();
			}

			return pBlock;
		};
	}
}}
