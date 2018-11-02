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

#include "MockMemoryBlockStorage.h"
#ifndef SIGNATURE_SCHEME_NIS1
#include "MockMemoryBlockStorage_data.h"
#else
#include "MockMemoryBlockStorage_data.nis1.h"
#endif
#include "catapult/model/BlockUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/Nemesis.h"
#include <memory.h>

namespace catapult { namespace mocks {

	MockMemoryBlockStorage::MockMemoryBlockStorage() {
		const auto* pNemesisBlock = reinterpret_cast<const model::Block*>(&mocks::MemoryBlockStorage_NemesisBlockData);
		auto nemesisBlockElement = test::BlockToBlockElement(*pNemesisBlock);
		nemesisBlockElement.GenerationHash = test::GetNemesisGenerationHash();
		saveBlock(nemesisBlockElement);
	}

	Height MockMemoryBlockStorage::chainHeight() const {
		return m_height;
	}

	namespace {
		void CopyHashes(model::TransactionElement& destElement, const model::TransactionElement& srcElement) {
			destElement.EntityHash = srcElement.EntityHash;
			destElement.MerkleComponentHash = srcElement.MerkleComponentHash;
		}

		std::shared_ptr<model::BlockElement> Copy(const model::Block& block, const model::BlockElement& blockElement) {
			auto pElement = std::make_shared<model::BlockElement>(block);
			pElement->EntityHash = blockElement.EntityHash;
			pElement->GenerationHash = blockElement.GenerationHash;
			pElement->SubCacheMerkleRoots = blockElement.SubCacheMerkleRoots;

			auto i = 0u;
			for (const auto& transaction : block.Transactions()) {
				pElement->Transactions.emplace_back(model::TransactionElement(transaction));
				CopyHashes(pElement->Transactions.back(), blockElement.Transactions[i]);
				++i;
			}

			return pElement;
		}
	}

	void MockMemoryBlockStorage::saveBlock(const model::BlockElement& blockElement) {
		auto height = blockElement.Block.Height;
		if (height != m_height + Height(1))
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot save out of order block at height", height);

		m_blocks[height] = test::CopyBlock(blockElement.Block);
		m_blockElements[height] = Copy(*m_blocks[height], blockElement);
		m_height = std::max(m_height, height);
	}

	std::shared_ptr<const model::Block> MockMemoryBlockStorage::loadBlock(Height height) const {
		if (height > m_height)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot load block at height greater than chain height", height);

		// reference elem, in case if it's not present, let the exception to be thrown
		return m_blocks.find(height)->second;
	}

	std::shared_ptr<const model::BlockElement> MockMemoryBlockStorage::loadBlockElement(Height height) const {
		if (height > m_height)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot load block at height greater than chain height", height);

		// reference elem, in case if it's not present, let the exception to be thrown
		return m_blockElements.find(height)->second;
	}

	model::HashRange MockMemoryBlockStorage::loadHashesFrom(Height height, size_t maxHashes) const {
		auto currentHeight = chainHeight();
		if (Height(0) == height || currentHeight < height)
			return model::HashRange();

		auto numAvailableHashes = static_cast<size_t>((currentHeight - height).unwrap() + 1);
		auto numHashes = std::min(maxHashes, numAvailableHashes);

		auto range = model::HashRange::PrepareFixed(numHashes);
		auto rangeIter = range.begin();
		for (auto i = 0u; i < numHashes; ++i)
			*rangeIter++ = m_blockElements.find(height + Height(i))->second->EntityHash;

		return range;
	}

	void MockMemoryBlockStorage::dropBlocksAfter(Height height) {
		m_height = height;
	}

	std::unique_ptr<io::BlockStorage> CreateMemoryBlockStorage(uint32_t numBlocks) {
		auto pStorage = std::make_unique<MockMemoryBlockStorage>();

		// storage already contains nemesis block (height 1)
		for (auto i = 2u; i <= numBlocks; ++i) {
			model::Block block;
			block.Size = sizeof(model::Block);
			block.Height = Height(i);
			pStorage->saveBlock(test::BlockToBlockElement(block));
		}

		return std::move(pStorage);
	}

	std::unique_ptr<io::BlockStorageCache> CreateMemoryBlockStorageCache(uint32_t numBlocks) {
		return std::make_unique<io::BlockStorageCache>(CreateMemoryBlockStorage(numBlocks));
	}
}}
