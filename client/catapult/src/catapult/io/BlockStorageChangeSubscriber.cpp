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
