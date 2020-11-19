/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#pragma once
#include "catapult/cache/CatapultCache.h"
#include "catapult/chain/ExecutionConfiguration.h"
#include "catapult/model/Block.h"
#include "catapult/model/BlockChainConfiguration.h"

namespace catapult { namespace harvesting {

	/// Importance block hash supplier.
	using ImportanceBlockHashSupplier = std::function<Hash256 (Height)>;

	/// Facade around unconfirmed transactions cache and updater.
	class HarvestingUtFacade {
	public:
		/// Creates a facade around \a blockTime, \a cache, \a blockChainConfig, \a executionConfig and \a importanceBlockHashSupplier.
		HarvestingUtFacade(
				Timestamp blockTime,
				const cache::CatapultCache& cache,
				const model::BlockChainConfiguration& blockChainConfig,
				const chain::ExecutionConfiguration& executionConfig,
				const ImportanceBlockHashSupplier& importanceBlockHashSupplier);

		/// Destroys the facade.
		~HarvestingUtFacade();

	public:
		/// Gets the locked height.
		Height height() const;

		/// Gets the number of successfully applied transactions.
		size_t size() const;

		/// Gets all successfully applied transactions.
		const std::vector<model::TransactionInfo>& transactionInfos() const;

	public:
		/// Attempts to apply \a transactionInfo to the cache.
		bool apply(const model::TransactionInfo& transactionInfo);

		/// Unapplies last successfully applied transaction.
		void unapply();

		/// Commits all transactions into a block with specified seed header (\a blockHeader).
		std::unique_ptr<model::Block> commit(const model::BlockHeader& blockHeader);

	private:
		class Impl;

	private:
		std::vector<model::TransactionInfo> m_transactionInfos;
		std::unique_ptr<Impl> m_pImpl;
	};

	/// Factory for creating unconfirmed transactions facades.
	class HarvestingUtFacadeFactory {
	public:
		/// Creates a factory around \a cache, \a blockChainConfig, \a executionConfig and \a importanceBlockHashSupplier.
		HarvestingUtFacadeFactory(
				const cache::CatapultCache& cache,
				const model::BlockChainConfiguration& blockChainConfig,
				const chain::ExecutionConfiguration& executionConfig,
				const ImportanceBlockHashSupplier& importanceBlockHashSupplier);

	public:
		/// Creates a facade for applying transactions at a given block time (\a blockTime).
		std::unique_ptr<HarvestingUtFacade> create(Timestamp blockTime) const;

	private:
		const cache::CatapultCache& m_cache;
		model::BlockChainConfiguration m_blockChainConfig;
		chain::ExecutionConfiguration m_executionConfig;
		ImportanceBlockHashSupplier m_importanceBlockHashSupplier;
	};
}}
