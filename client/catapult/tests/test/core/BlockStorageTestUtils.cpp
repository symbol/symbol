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

#include "BlockStorageTestUtils.h"
#include "BlockTestUtils.h"
#include "catapult/io/BlockStatementSerializer.h"
#include "catapult/io/BufferInputStreamAdapter.h"

namespace catapult { namespace test {

	void SeedBlocks(io::BlockStorage& storage, Height startHeight, Height endHeight) {
		for (auto height = startHeight; height <= endHeight; height = height + Height(1)) {
			auto pBlock = GenerateBlockWithTransactions(5, height);
			auto blockElement = BlockToBlockElement(*pBlock);
			blockElement.EntityHash = Hash256();
			blockElement.EntityHash[Hash256::Size - 1] = static_cast<uint8_t>(height.unwrap());
			storage.saveBlock(blockElement);
		}
	}

	void SeedBlocks(io::BlockStorage& storage, size_t numBlocks) {
		SeedBlocks(storage, Height(2), Height(numBlocks));
	}

	model::BlockElement CreateBlockElementForSaveTests(const model::Block& block) {
		// Arrange: create a block element with a random hash
		auto blockElement = BlockToBlockElement(block, GenerateRandomByteArray<Hash256>());
		blockElement.GenerationHash = GenerateRandomByteArray<GenerationHash>();

		// - give the first transaction a random hash too (the random hash should be saved)
		blockElement.Transactions[0].EntityHash = GenerateRandomByteArray<Hash256>();
		return blockElement;
	}

	std::shared_ptr<const model::BlockElement> LoadBlockElementWithStatements(const io::BlockStorage& storage, Height height) {
		auto pBlockElement = storage.loadBlockElement(height);
		auto blockStatementPair = storage.loadBlockStatementData(height);

		if (blockStatementPair.second) {
			auto pBlockStatement = std::make_shared<model::BlockStatement>();
			io::BufferInputStreamAdapter<std::vector<uint8_t>> blockStatementStream(blockStatementPair.first);
			io::ReadBlockStatement(blockStatementStream, *pBlockStatement);
			const_cast<model::BlockElement&>(*pBlockElement).OptionalStatement = std::move(pBlockStatement);
		}

		return pBlockElement;
	}
}}
