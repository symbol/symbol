#include "BlockStorageChangeSubscriber.h"
#include "BlockStorage.h"

namespace catapult { namespace io {

	namespace {
		class BlockStorageChangeSubscriber : public BlockChangeSubscriber {
		public:
			explicit BlockStorageChangeSubscriber(std::unique_ptr<LightBlockStorage>&& pStorage) : m_pStorage(std::move(pStorage))
			{}

		public:
			void notifyBlock(const model::BlockElement& blockElement) override {
				m_pStorage->saveBlock(blockElement);
			}

			void notifyDropBlocksAfter(Height height) override {
				m_pStorage->dropBlocksAfter(height);
			}

		private:
			std::unique_ptr<LightBlockStorage> m_pStorage;
		};
	}

	std::unique_ptr<BlockChangeSubscriber> CreateBlockStorageChangeSubscriber(std::unique_ptr<LightBlockStorage>&& pStorage) {
		return std::make_unique<BlockStorageChangeSubscriber>(std::move(pStorage));
	}
}}
