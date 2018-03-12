#include "MockMemoryBasedStorage.h"
#ifndef SIGNATURE_SCHEME_NIS1
#include "MockMemoryBasedStorage_data.h"
#else
#include "MockMemoryBasedStorage_data.nis1.h"
#endif
#include "catapult/model/BlockUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/Nemesis.h"
#include <memory.h>

namespace catapult { namespace mocks {

	MockMemoryBasedStorage::MockMemoryBasedStorage() {
		const auto* pNemesisBlock = reinterpret_cast<const model::Block*>(&mocks::MemoryBasedStorage_NemesisBlockData);
		auto nemesisBlockElement = test::BlockToBlockElement(*pNemesisBlock);
		nemesisBlockElement.GenerationHash = test::GetNemesisGenerationHash();
		saveBlock(nemesisBlockElement);
	}

	Height MockMemoryBasedStorage::chainHeight() const {
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

			auto i = 0u;
			for (const auto& transaction : block.Transactions()) {
				pElement->Transactions.emplace_back(model::TransactionElement(transaction));
				CopyHashes(pElement->Transactions.back(), blockElement.Transactions[i]);
				++i;
			}

			return pElement;
		}
	}

	void MockMemoryBasedStorage::saveBlock(const model::BlockElement& blockElement) {
		auto height = blockElement.Block.Height;
		if (height != m_height + Height(1))
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot save out of order block at height", height);

		m_blocks[height] = test::CopyBlock(blockElement.Block);
		m_blockElements[height] = Copy(*m_blocks[height], blockElement);
		m_height = std::max(m_height, height);
	}

	std::shared_ptr<const model::Block> MockMemoryBasedStorage::loadBlock(Height height) const {
		if (height > m_height)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot load block at height greater than chain height", height);

		// reference elem, in case if it's not present, let the exception to be thrown
		return m_blocks.find(height)->second;
	}

	std::shared_ptr<const model::BlockElement> MockMemoryBasedStorage::loadBlockElement(Height height) const {
		if (height > m_height)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot load block at height greater than chain height", height);

		// reference elem, in case if it's not present, let the exception to be thrown
		return m_blockElements.find(height)->second;
	}

	model::HashRange MockMemoryBasedStorage::loadHashesFrom(Height height, size_t maxHashes) const {
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

	void MockMemoryBasedStorage::dropBlocksAfter(Height height) {
		m_height = height;
	}

	std::unique_ptr<io::BlockStorage> CreateMemoryBasedStorage(uint32_t numBlocks) {
		auto pStorage = std::make_unique<MockMemoryBasedStorage>();

		// storage already contains nemesis block (height 1)
		for (auto i = 2u; i <= numBlocks; ++i) {
			model::Block block;
			block.Size = sizeof(model::Block);
			block.Height = Height(i);
			pStorage->saveBlock(test::BlockToBlockElement(block));
		}

		return std::move(pStorage);
	}

	std::unique_ptr<io::BlockStorageCache> CreateMemoryBasedStorageCache(uint32_t numBlocks) {
		return std::make_unique<io::BlockStorageCache>(CreateMemoryBasedStorage(numBlocks));
	}
}}
