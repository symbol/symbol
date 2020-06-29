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

#include "BlockStorageCache.h"
#include "MoveBlockFiles.h"
#include "catapult/model/Elements.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace io {

	namespace {
		std::shared_ptr<const model::Block> BlockElementAsSharedBlock(const std::shared_ptr<const model::BlockElement>& pBlockElement) {
			return std::shared_ptr<const model::Block>(&pBlockElement->Block, [pBlockElement](const auto*) {});
		}
	}

	// region CachedData

	struct CachedData {
	public:
		Height height() const {
			return m_pBlockElement ? m_pBlockElement->Block.Height : Height(0);
		}

		std::shared_ptr<const model::Block> block(Height) const {
			return BlockElementAsSharedBlock(m_pBlockElement);
		}

		std::shared_ptr<const model::BlockElement> blockElement(Height) const {
			return m_pBlockElement;
		}

	public:
		bool contains(Height height) const {
			return height == m_pBlockElement->Block.Height;
		}

	public:
		void update(const std::shared_ptr<const model::BlockElement>& pBlockElement) {
			m_pBlockElement = pBlockElement;
		}

		void reset() {
			m_pBlockElement.reset();
		}

	private:
		std::shared_ptr<const model::BlockElement> m_pBlockElement;
	};

	// endregion

	// region BlockStorageView

	BlockStorageView::BlockStorageView(
			const BlockStorage& storage,
			utils::SpinReaderWriterLock::ReaderLockGuard&& readLock,
			const CachedData& cachedData)
			: m_storage(storage)
			, m_readLock(std::move(readLock))
			, m_cachedData(cachedData)
	{}

	Height BlockStorageView::chainHeight() const {
		return m_cachedData.height();
	}

	model::HashRange BlockStorageView::loadHashesFrom(Height height, size_t maxHashes) const {
		return m_storage.loadHashesFrom(height, maxHashes);
	}

	std::shared_ptr<const model::Block> BlockStorageView::loadBlock(Height height) const {
		requireHeight(height, "block");
		if (m_cachedData.contains(height))
			return m_cachedData.block(height);

		return m_storage.loadBlock(height);
	}

	std::shared_ptr<const model::BlockElement> BlockStorageView::loadBlockElement(Height height) const {
		requireHeight(height, "block element");
		if (m_cachedData.contains(height))
			return m_cachedData.blockElement(height);

		return m_storage.loadBlockElement(height);
	}

	std::pair<std::vector<uint8_t>, bool> BlockStorageView::loadBlockStatementData(Height height) const {
		requireHeight(height, "block statement data");
		return m_storage.loadBlockStatementData(height);
	}

	void BlockStorageView::requireHeight(Height height, const char* description) const {
		auto chainHeight = this->chainHeight();
		if (height <= chainHeight)
			return;

		std::ostringstream out;
		out << "cannot load " << description << " at height (" << height << ") greater than chain height (" << chainHeight << ")";
		CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
	}

	// endregion

	// region BlockStorageModifier

	BlockStorageModifier::BlockStorageModifier(
			BlockStorage& storage,
			PrunableBlockStorage& stagingStorage,
			utils::SpinReaderWriterLock::WriterLockGuard&& writeLock,
			CachedData& cachedData)
			: m_storage(storage)
			, m_stagingStorage(stagingStorage)
			, m_writeLock(std::move(writeLock))
			, m_cachedData(cachedData) {
		dropBlocksAfter(storage.chainHeight());
	}

	void BlockStorageModifier::saveBlock(const model::BlockElement& blockElement) {
		m_stagingStorage.saveBlock(blockElement);
	}

	void BlockStorageModifier::saveBlocks(const std::vector<model::BlockElement>& blockElements) {
		for (const auto& blockElement : blockElements)
			m_stagingStorage.saveBlock(blockElement);
	}

	void BlockStorageModifier::dropBlocksAfter(Height height) {
		m_stagingStorage.dropBlocksAfter(height);
		m_saveStartHeight = height;
	}

	void BlockStorageModifier::commit() {
		// 1. apply staging changes to permananent storage
		MoveBlockFiles(m_stagingStorage, m_storage, m_saveStartHeight + Height(1));

		// 2. update cache
		auto newChainHeight = m_storage.chainHeight();
		if (newChainHeight > Height(0))
			m_cachedData.update(m_storage.loadBlockElement(newChainHeight));
		else
			m_cachedData.reset();
	}

	// endregion

	// region BlockStorageCache

	BlockStorageCache::BlockStorageCache(std::unique_ptr<BlockStorage>&& pStorage, std::unique_ptr<PrunableBlockStorage>&& pStagingStorage)
			: m_pStorage(std::move(pStorage))
			, m_pStagingStorage(std::move(pStagingStorage))
			, m_pCachedData(std::make_unique<CachedData>()) {
		m_pCachedData->update(m_pStorage->loadBlockElement(m_pStorage->chainHeight()));
	}

	BlockStorageCache::~BlockStorageCache() = default;

	BlockStorageView BlockStorageCache::view() const {
		auto readLock = m_lock.acquireReader();
		return BlockStorageView(*m_pStorage, std::move(readLock), *m_pCachedData);
	}

	BlockStorageModifier BlockStorageCache::modifier() {
		auto writeLock = m_lock.acquireWriter();
		return BlockStorageModifier(*m_pStorage, *m_pStagingStorage, std::move(writeLock), *m_pCachedData);
	}

	// endregion
}}
