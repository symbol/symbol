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
#include "BlockStorage.h"
#include "FixedSizeValueStorage.h"
#include "IndexFile.h"
#include "RawFile.h"
#include <string>

namespace catapult { namespace io {

	/// File block storage modes.
	enum class FileBlockStorageMode {
		/// Maintain hash-based index.
		Hash_Index,

		/// None.
		None
	};

	/// File-based block storage.
	class FileBlockStorage final : public PrunableBlockStorage {
	public:
		/// Creates a file-based block storage, where blocks will be stored inside \a dataDirectory
		/// with specified storage \a mode.
		explicit FileBlockStorage(const std::string& dataDirectory, FileBlockStorageMode mode = FileBlockStorageMode::Hash_Index);

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
		std::string m_dataDirectory;
		FileBlockStorageMode m_mode;

		HashFile m_hashFile;
		IndexFile m_indexFile;
	};
}}
