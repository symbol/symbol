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
#include "Accounts.h"
#include "StateHashCalculator.h"
#include "TransactionsGenerator.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/state/BlockStatistic.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include <set>

namespace catapult { namespace test {

	/// Single use builder used to build a single block or block chain from transactions.
	class BlockChainBuilder {
	public:
		using Blocks = std::vector<std::shared_ptr<model::Block>>;
		using BlockReceiptsHashCalculator = std::function<Hash256 (const model::Block&)>;

	public:
		/// Creates a builder around \a accounts and \a stateHashCalculator.
		BlockChainBuilder(const Accounts& accounts, StateHashCalculator& stateHashCalculator);

		/// Creates a builder around \a accounts, \a stateHashCalculator and \a config.
		BlockChainBuilder(
				const Accounts& accounts,
				StateHashCalculator& stateHashCalculator,
				const model::BlockChainConfiguration& config);

		/// Creates a builder around \a accounts, \a stateHashCalculator, \a config and explicit \a resourcesPath.
		BlockChainBuilder(
				const Accounts& accounts,
				StateHashCalculator& stateHashCalculator,
				const model::BlockChainConfiguration& config,
				const std::string& resourcesPath);

	private:
		BlockChainBuilder(
				const Accounts& accounts,
				StateHashCalculator& stateHashCalculator,
				const model::BlockChainConfiguration& config,
				const std::string& resourcesPath,
				bool isChained);

	public:
		/// Sets the time between blocks to \a blockTimeInterval.
		void setBlockTimeInterval(utils::TimeSpan blockTimeInterval);

		/// Sets a custom block receipts hash calculator (\a blockReceiptsHashCalculator).
		void setBlockReceiptsHashCalculator(const BlockReceiptsHashCalculator& blockReceiptsHashCalculator);

		/// Creates a new builder starting at this builder's terminal block.
		BlockChainBuilder createChainedBuilder();

		/// Creates a new builder starting at this builder's terminal block with a different
		/// state hash calculator (\a stateHashCalculator).
		BlockChainBuilder createChainedBuilder(StateHashCalculator& stateHashCalculator) const;

		/// Creates a new builder with a different state hash calculator (\a stateHashCalculator) starting at the supplied \a block.
		BlockChainBuilder createChainedBuilder(StateHashCalculator& stateHashCalculator, const model::Block& block) const;

	public:
		/// Builds a single block with transactions from \a transactionsGenerator.
		std::unique_ptr<model::Block> asSingleBlock(const TransactionsGenerator& transactionsGenerator);

		/// Builds a block chain with transactions from \a transactionsGenerator.
		Blocks asBlockChain(const TransactionsGenerator& transactionsGenerator);

	private:
		void pushDifficulty(const model::Block& block);

		std::unique_ptr<model::Block> createBlock(
				const model::PreviousBlockContext& context,
				Timestamp timestamp,
				const model::Transactions& transactions);

		crypto::KeyPair findBlockSigner(const model::PreviousBlockContext& context, Timestamp timestamp, Difficulty difficulty);

	private:
		static std::shared_ptr<const model::BlockElement> ToSharedBlockElement(const model::Block& block);

	private:
		// pointers instead of references to allow copy
		const Accounts* m_pAccounts;
		StateHashCalculator* m_pStateHashCalculator;

		std::shared_ptr<const model::BlockElement> m_pParentBlockElement;
		std::shared_ptr<const model::BlockElement> m_pTailBlockElement;
		std::shared_ptr<const model::Block> m_pNemesisBlock; // only used to extend block lifetime
		std::set<state::BlockStatistic> m_statistics;
		Hash256 m_previousImportanceBlockHash;

		utils::TimeSpan m_blockTimeInterval;
		BlockReceiptsHashCalculator m_blockReceiptsHashCalculator;
		model::BlockChainConfiguration m_config;
	};
}}
