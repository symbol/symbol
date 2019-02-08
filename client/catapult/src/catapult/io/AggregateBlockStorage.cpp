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
			// region LightBlockStorage

			Height chainHeight() const override {
				return m_pStorage->chainHeight();
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

			// endregion

			// region BlockStorage

			std::shared_ptr<const model::Block> loadBlock(Height height) const override {
				return m_pStorage->loadBlock(height);
			}

			std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const override {
				return m_pStorage->loadBlockElement(height);
			}

			std::pair<std::vector<uint8_t>, bool> loadBlockStatementData(Height height) const override {
				return m_pStorage->loadBlockStatementData(height);
			}

			// endregion

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
