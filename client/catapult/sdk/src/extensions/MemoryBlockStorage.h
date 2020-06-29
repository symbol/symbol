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
#include "catapult/model/Elements.h"
#include <map>

namespace catapult { namespace extensions {

	/// Memory-based block storage that loads and saves blocks in memory.
	class MemoryBlockStorage : public io::PrunableBlockStorage {
	private:
		using Blocks = std::map<Height, std::shared_ptr<model::Block>>;
		using BlockElements = std::map<Height, std::shared_ptr<model::BlockElement>>;
		using BlockStatements = std::map<Height, std::shared_ptr<const model::BlockStatement>>;

	public:
		/// Creates a memory-based block storage around \a nemesisBlockElement.
		explicit MemoryBlockStorage(const model::BlockElement& nemesisBlockElement);

	public:
		// LightBlockStorage
		Height chainHeight() const override;
		model::HashRange loadHashesFrom(Height height, size_t maxHashes) const override;
		void saveBlock(const model::BlockElement& blockElement) override;
		void dropBlocksAfter(Height height) override;

		// BlockStorage
		std::shared_ptr<const model::Block> loadBlock(Height height) const override;
		std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const override;
		std::pair<std::vector<uint8_t>, bool> loadBlockStatementData(Height height) const override;

		// PrunableBlockStorage
		void purge() override;

	private:
		void requireHeight(Height height, const char* description) const;

	private:
		Blocks m_blocks;
		BlockElements m_blockElements;
		BlockStatements m_blockStatements;
		Height m_height;
	};
}}
