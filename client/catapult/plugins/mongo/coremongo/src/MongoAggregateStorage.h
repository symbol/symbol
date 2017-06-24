#pragma once
#include "catapult/io/BlockStorage.h"
#include "catapult/types.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Aggregate around mongo and file storage.
	class MongoAggregateStorage : public io::BlockStorage {
	public:
		/// Creates aggregate around \a pMongoStorage and \a pFileStorage.
		/// File storage will keep at most \a maxRollbackBlocks.
		MongoAggregateStorage(
				std::shared_ptr<io::LightBlockStorage>&& pMongoStorage,
				std::shared_ptr<io::PrunableBlockStorage>&& pFileStorage,
				uint32_t maxRollbackBlocks);

	public:
		Height chainHeight() const override;

		std::shared_ptr<const model::Block> loadBlock(Height height) const override;

		std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const override;

		model::HashRange loadHashesFrom(Height height, size_t maxHashes) const override;

		void saveBlock(const model::BlockElement& blockElement) override;

		void dropBlocksAfter(Height height) override;

	private:
		std::shared_ptr<io::LightBlockStorage> m_pMongoStorage;
		std::shared_ptr<io::PrunableBlockStorage> m_pFileStorage;
		uint32_t m_maxRollbackBlocks;
	};
}}}
