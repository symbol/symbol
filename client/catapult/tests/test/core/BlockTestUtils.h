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

#pragma once
#include "TransactionTestUtils.h"
#include "catapult/model/Elements.h"
#include "catapult/model/RangeTypes.h"
#include <list>
#include <memory>
#include <vector>

namespace catapult { namespace test {

	/// Hash string of the deterministic block.
	constexpr auto Deterministic_Block_Hash_String = "4E081D2CD7180E58BEEF2745895BCC18DF26F75F4DF2CDB7F057C5E169F2E61C";

	// region TestBlockTransactions

	/// Container of transactions for seeding a test block.
	class TestBlockTransactions {
	public:
		/// Creates block transactions from const \a transactions.
		TestBlockTransactions(const ConstTransactions& transactions);

		/// Creates block transactions from mutable \a transactions.
		TestBlockTransactions(const MutableTransactions& transactions);

		/// Creates \a numTransactions (random) block transactions.
		TestBlockTransactions(size_t numTransactions);

	public:
		/// Gets the transactions.
		const ConstTransactions& get() const;

	private:
		ConstTransactions m_transactions;
	};

	// endregion

	// region Block factory functions

	/// Generates an empty block with random signer and no transactions.
	std::unique_ptr<model::Block> GenerateEmptyRandomBlock();

	/// Generates a block with random signer and \a transactions.
	std::unique_ptr<model::Block> GenerateBlockWithTransactions(const TestBlockTransactions& transactions);

	/// Generates a block with a given \a signer and \a transactions.
	std::unique_ptr<model::Block> GenerateBlockWithTransactions(const crypto::KeyPair& signer, const TestBlockTransactions& transactions);

	/// Generates a block with \a numTransactions transactions at \a height.
	std::unique_ptr<model::Block> GenerateBlockWithTransactions(size_t numTransactions, Height height);

	/// Generates a block with \a numTransactions transactions at \a height and \a timestamp.
	std::unique_ptr<model::Block> GenerateBlockWithTransactions(size_t numTransactions, Height height, Timestamp timestamp);

	/// Generates a predefined block, i.e. this function will always return the same block.
	std::unique_ptr<model::Block> GenerateDeterministicBlock();

	// endregion

	/// Creates a buffer containing \a numBlocks random blocks (all with no transactions).
	std::vector<uint8_t> CreateRandomBlockBuffer(size_t numBlocks);

	/// Copies \a blocks into an entity range.
	model::BlockRange CreateEntityRange(const std::vector<const model::Block*>& blocks);

	/// Creates a block entity range composed of \a numBlocks blocks.
	model::BlockRange CreateBlockEntityRange(size_t numBlocks);

	/// Creates \a count ranges of blocks.
	std::vector<model::BlockRange> PrepareRanges(size_t count);

	/// Counts the number of transactions in \a block.
	size_t CountTransactions(const model::Block& block);

	/// Converts \a block to a block element.
	model::BlockElement BlockToBlockElement(const model::Block& block);

	/// Converts \a block to a block element with specified generation hash seed (\a generationHashSeed).
	model::BlockElement BlockToBlockElement(const model::Block& block, const GenerationHashSeed& generationHashSeed);

	/// Converts \a block with \a hash to a block element.
	model::BlockElement BlockToBlockElement(const model::Block& block, const Hash256& hash);

	/// Verifies that block elements \a expectedBlockElement and \a blockElement are equivalent.
	void AssertEqual(const model::BlockElement& expectedBlockElement, const model::BlockElement& blockElement);
}}
