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
#include "catapult/model/Elements.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace io {

	namespace {
		std::shared_ptr<const model::Block> BlockElementAsSharedBlock(const std::shared_ptr<const model::BlockElement>& pBlockElement) {
			return std::shared_ptr<const model::Block>(&pBlockElement->Block, [pBlockElement](const auto*) {});
		}

		void CopyHashes(model::TransactionElement& destElement, const model::TransactionElement& srcElement) {
			destElement.EntityHash = srcElement.EntityHash;
			destElement.MerkleComponentHash = srcElement.MerkleComponentHash;
		}

		std::shared_ptr<model::BlockElement> Copy(const model::BlockElement& originalBlockElement) {
			using model::BlockElement;

			auto dataSize = sizeof(BlockElement) + originalBlockElement.Block.Size;
			auto pData = utils::MakeUniqueWithSize<uint8_t>(dataSize);

			// copy the block data
			auto pBlockData = pData.get() + sizeof(BlockElement);
			std::memcpy(pBlockData, &originalBlockElement.Block, originalBlockElement.Block.Size);

			// create the block element and transfer ownership from pData to pBlockElement
			auto pBlockElementRaw = new (pData.get()) BlockElement(*reinterpret_cast<model::Block*>(pBlockData));
			auto pBlockElement = std::shared_ptr<BlockElement>(pBlockElementRaw);
			pData.release();

			pBlockElement->EntityHash = originalBlockElement.EntityHash;
			pBlockElement->GenerationHash = originalBlockElement.GenerationHash;
			pBlockElement->SubCacheMerkleRoots = originalBlockElement.SubCacheMerkleRoots;

			auto i = 0u;
			for (const auto& transaction : pBlockElement->Block.Transactions()) {
				pBlockElement->Transactions.emplace_back(model::TransactionElement(transaction));
				CopyHashes(pBlockElement->Transactions.back(), originalBlockElement.Transactions[i]);
				++i;
			}

			return pBlockElement;
		}
	}

	/// Cached data holder.
	struct CachedData {
	public:
		CachedData() = default;

		/// Returns cached height.
		Height getHeight() const {
			return m_chainHeight;
		}

		/// Returns \c true if cache contains element at \a height.
		bool contains(Height height) const {
			return m_pBlockElement && height == m_pBlockElement->Block.Height;
		}

		/// Returns cached block element.
		std::shared_ptr<const model::BlockElement> getBlockElement(Height) const {
			return m_pBlockElement;
		}

		/// Returns cached block.
		std::shared_ptr<const model::Block> getBlock(Height) const {
			return BlockElementAsSharedBlock(m_pBlockElement);
		}

		/// Updates cache with block element (\a blockElement).
		void update(const model::BlockElement& blockElement) {
			// note: update receives elements during loadBlock, but also saveBlock. We get them from BlockChainSyncConsumer,
			// and it gets them from disruptor... in order NOT to copy here we'd need to take ownership of those.
			// Currently we can't/shouldn't do it, as there's "new block" consumer afterwards and possibly ProcessingCompleteFunc.
			m_pBlockElement = Copy(blockElement);
		}

		/// Updates cached height to \a height.
		void update(Height height) {
			m_chainHeight = height;

			if (m_pBlockElement && height < m_pBlockElement->Block.Height)
				m_pBlockElement.reset();
		}

	private:
		// note: the reason to have them separated is drop blocks, which
		// updates the height, but we don't want to touch cached block(s).
		Height m_chainHeight;
		std::shared_ptr<const model::BlockElement> m_pBlockElement;
	};

	BlockStorageCache::~BlockStorageCache() = default;

	// This ctor takes r-value, to move the storage (that's not a move ctor).
	BlockStorageCache::BlockStorageCache(std::unique_ptr<BlockStorage>&& pStorage)
			: m_pStorage(std::move(pStorage))
			, m_pCachedData(std::make_unique<CachedData>()) {
		m_pCachedData->update(m_pStorage->chainHeight());
	}

	// region BlockStorageView

	Height BlockStorageView::chainHeight() const {
		return m_cachedData.getHeight();
	}

	std::shared_ptr<const model::Block> BlockStorageView::loadBlock(Height height) const {
		if (height > chainHeight())
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot load block at height greater than chain height", height);

		if (m_cachedData.contains(height))
			return m_cachedData.getBlock(height);

		return m_storage.loadBlock(height);
	}

	std::shared_ptr<const model::BlockElement> BlockStorageView::loadBlockElement(Height height) const {
		if (height > chainHeight())
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot load block at height greater than chain height", height);

		if (m_cachedData.contains(height))
			return m_cachedData.getBlockElement(height);

		return m_storage.loadBlockElement(height);
	}

	model::HashRange BlockStorageView::loadHashesFrom(Height height, size_t maxHashes) const {
		return m_storage.loadHashesFrom(height, maxHashes);
	}

	// endregion

	// region BlockStorageModifier

	namespace {
		void CacheBlockElement(CachedData& cachedData, const model::BlockElement& blockElement) {
			if (blockElement.Block.Height > cachedData.getHeight())
				cachedData.update(blockElement.Block.Height);

			cachedData.update(blockElement);
		}
	}

	void BlockStorageModifier::saveBlock(const model::BlockElement& blockElement) {
		m_storage.saveBlock(blockElement);
		CacheBlockElement(m_cachedData, blockElement);
	}

	void BlockStorageModifier::saveBlocks(const std::vector<model::BlockElement>& blockElements) {
		if (blockElements.empty())
			return;

		for (const auto& blockElement : blockElements)
			m_storage.saveBlock(blockElement);

		CacheBlockElement(m_cachedData, blockElements.back());
	}

	void BlockStorageModifier::dropBlocksAfter(Height height) {
		m_storage.dropBlocksAfter(height);
		m_cachedData.update(height);
	}

	// endregion

	// region BlockStorageCache

	BlockStorageView BlockStorageCache::view() const {
		return BlockStorageView(*m_pStorage, m_lock.acquireReader(), *m_pCachedData);
	}

	BlockStorageModifier BlockStorageCache::modifier() {
		return BlockStorageModifier(*m_pStorage, m_lock.acquireReader(), *m_pCachedData);
	}

	// endregion
}}
