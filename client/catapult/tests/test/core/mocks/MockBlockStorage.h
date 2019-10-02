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

#pragma once
#include "catapult/io/BlockStorage.h"

namespace catapult { namespace mocks {

	// region UnsupportedBlockStorage

	/// Unsupported block storage.
	class UnsupportedBlockStorage : public io::BlockStorage {
	public:
		Height chainHeight() const override {
			CATAPULT_THROW_RUNTIME_ERROR("chainHeight - not supported in mock");
		}

		model::HashRange loadHashesFrom(Height, size_t) const override {
			CATAPULT_THROW_RUNTIME_ERROR("loadHashesFrom - not supported in mock");
		}

		void saveBlock(const model::BlockElement&) override {
			CATAPULT_THROW_RUNTIME_ERROR("saveBlock - not supported in mock");
		}

		void dropBlocksAfter(Height) override {
			CATAPULT_THROW_RUNTIME_ERROR("dropBlocksAfter - not supported in mock");
		}

		std::shared_ptr<const model::Block> loadBlock(Height) const override {
			CATAPULT_THROW_RUNTIME_ERROR("loadBlock - not supported in mock");
		}

		std::shared_ptr<const model::BlockElement> loadBlockElement(Height) const override {
			CATAPULT_THROW_RUNTIME_ERROR("loadBlockElement - not supported in mock");
		}

		std::pair<std::vector<uint8_t>, bool> loadBlockStatementData(Height) const override {
			CATAPULT_THROW_RUNTIME_ERROR("loadBlockStatementData - not supported in mock");
		}
	};

	// endregion

	// region MockHeightOnlyBlockStorage

	/// Mock block storage that only supports chain height accesses.
	class MockHeightOnlyBlockStorage : public UnsupportedBlockStorage {
	public:
		/// Creates the storage with height \a chainHeight.
		explicit MockHeightOnlyBlockStorage(Height chainHeight) : m_chainHeight(chainHeight)
		{}

	public:
		Height chainHeight() const override {
			return m_chainHeight;
		}

	private:
		Height m_chainHeight;
	};

	// endregion

	// region MockSavingBlockStorage

	/// Mock block storage that captures saved blocks.
	class MockSavingBlockStorage : public MockHeightOnlyBlockStorage {
	public:
		using MockHeightOnlyBlockStorage::MockHeightOnlyBlockStorage;

	public:
		/// Gets all saved block elements.
		const std::vector<model::BlockElement>& savedBlockElements() const {
			return m_savedBlockElements;
		}

	public:
		void saveBlock(const model::BlockElement& blockElement) override {
			m_savedBlockElements.push_back(blockElement);
		}

	private:
		std::vector<model::BlockElement> m_savedBlockElements;
	};

	// endregion
}}
