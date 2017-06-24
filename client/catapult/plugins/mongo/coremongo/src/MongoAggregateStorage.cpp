#include "MongoAggregateStorage.h"

namespace catapult { namespace mongo { namespace plugins {

	MongoAggregateStorage::MongoAggregateStorage(
			std::shared_ptr<io::LightBlockStorage>&& pMongoStorage,
			std::shared_ptr<io::PrunableBlockStorage>&& pFileStorage,
			uint32_t maxRollbackBlocks)
			: m_pMongoStorage(std::move(pMongoStorage))
			, m_pFileStorage(std::move(pFileStorage))
			, m_maxRollbackBlocks(maxRollbackBlocks)
	{}

	Height MongoAggregateStorage::chainHeight() const {
		return m_pMongoStorage->chainHeight();
	}

	std::shared_ptr<const model::Block> MongoAggregateStorage::loadBlock(Height height) const {
		return m_pFileStorage->loadBlock(height);
	}

	std::shared_ptr<const model::BlockElement> MongoAggregateStorage::loadBlockElement(Height height) const {
		return m_pFileStorage->loadBlockElement(height);
	}

	model::HashRange MongoAggregateStorage::loadHashesFrom(Height height, size_t maxHashes) const {
		return m_pMongoStorage->loadHashesFrom(height, maxHashes);
	}

	void MongoAggregateStorage::saveBlock(const model::BlockElement& blockElement) {
		if (Height(1) != blockElement.Block.Height)
			m_pFileStorage->saveBlock(blockElement);

		if (blockElement.Block.Height.unwrap() > m_maxRollbackBlocks)
			m_pFileStorage->pruneBlocksBefore(blockElement.Block.Height - Height(m_maxRollbackBlocks));

		m_pMongoStorage->saveBlock(blockElement);
	}

	void MongoAggregateStorage::dropBlocksAfter(Height height) {
		m_pFileStorage->dropBlocksAfter(height);
		m_pMongoStorage->dropBlocksAfter(height);
	}
}}}
