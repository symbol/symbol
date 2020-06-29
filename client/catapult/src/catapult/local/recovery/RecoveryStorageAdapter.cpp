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

#include "RecoveryStorageAdapter.h"

namespace catapult { namespace local {

	namespace {
		struct ReadOnlyStorageAdapter : public io::BlockStorage {
		public:
			explicit ReadOnlyStorageAdapter(const io::BlockStorage& storage) : m_storage(storage)
			{}

		public:
			Height chainHeight() const override {
				return m_storage.chainHeight();
			}

			model::HashRange loadHashesFrom(Height height, size_t maxHashes) const override {
				return m_storage.loadHashesFrom(height, maxHashes);
			}

			void saveBlock(const model::BlockElement&) override {
				CATAPULT_THROW_RUNTIME_ERROR("saveBlock unsupported in recovery storage");
			}

			void dropBlocksAfter(Height) override {
				CATAPULT_THROW_RUNTIME_ERROR("dropBlocksAfter unsupported in recovery storage");
			}

		public:
			std::shared_ptr<const model::Block> loadBlock(Height height) const override {
				return m_storage.loadBlock(height);
			}

			std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const override {
				return m_storage.loadBlockElement(height);
			}

			std::pair<std::vector<uint8_t>, bool> loadBlockStatementData(Height height) const override {
				return m_storage.loadBlockStatementData(height);
			}

		private:
			const io::BlockStorage& m_storage;
		};
	}

	std::unique_ptr<io::BlockStorage> CreateReadOnlyStorageAdapter(const io::BlockStorage& storage) {
		return std::make_unique<ReadOnlyStorageAdapter>(storage);
	}
}}
