#include "AggregateBlockStorage.h"

namespace catapult { namespace io {

	namespace {
		class AggregateBlockStorage : public BlockStorage {
		public:
			AggregateBlockStorage(
					std::unique_ptr<BlockStorage>&& pStorage,
					std::unique_ptr<BlockChangeSubscriber>&& pBlockChangeSubscriber)
					: m_pStorage(std::move(pStorage))
					, m_pBlockChangeSubscriber(std::move(pBlockChangeSubscriber))
			{}

		public:
			Height chainHeight() const override {
				return m_pStorage->chainHeight();
			}

			std::shared_ptr<const model::Block> loadBlock(Height height) const override {
				return m_pStorage->loadBlock(height);
			}

			std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const override {
				return m_pStorage->loadBlockElement(height);
			}

			model::HashRange loadHashesFrom(Height height, size_t maxHashes) const override {
				return m_pStorage->loadHashesFrom(height, maxHashes);
			}

			void saveBlock(const model::BlockElement& blockElement) override {
				m_pStorage->saveBlock(blockElement);
				m_pBlockChangeSubscriber->notifyBlock(blockElement);
			}

			void dropBlocksAfter(Height height) override {
				m_pStorage->dropBlocksAfter(height);
				m_pBlockChangeSubscriber->notifyDropBlocksAfter(height);
			}

		private:
			std::unique_ptr<BlockStorage> m_pStorage;
			std::unique_ptr<BlockChangeSubscriber> m_pBlockChangeSubscriber;
		};
	}

	std::unique_ptr<BlockStorage> CreateAggregateBlockStorage(
			std::unique_ptr<BlockStorage>&& pStorage,
			std::unique_ptr<BlockChangeSubscriber>&& pBlockChangeSubscriber) {
		return std::make_unique<AggregateBlockStorage>(std::move(pStorage), std::move(pBlockChangeSubscriber));
	}
}}
