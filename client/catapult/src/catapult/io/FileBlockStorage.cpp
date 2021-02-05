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

#include "FileBlockStorage.h"
#include "BlockElementSerializer.h"
#include "BlockStatementSerializer.h"
#include "BufferedFileStream.h"
#include "FilesystemUtils.h"
#include "PodIoUtils.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/utils/MemoryUtils.h"
#include "catapult/preprocessor.h"

namespace catapult { namespace io {

	// region ctor

	FileBlockStorage::FileBlockStorage(const std::string& dataDirectory, uint32_t fileDatabaseBatchSize, FileBlockStorageMode mode)
			: m_dataDirectory(dataDirectory)
			, m_mode(mode)
			, m_blockDatabase(config::CatapultDirectory(dataDirectory), { fileDatabaseBatchSize, ".dat" })
			, m_statementDatabase(config::CatapultDirectory(dataDirectory), { fileDatabaseBatchSize, ".stmt" })
			, m_hashFile(dataDirectory, "hashes")
			, m_indexFile((std::filesystem::path(dataDirectory) / "index.dat").generic_string())
	{}

	// endregion

	// region LightBlockStorage

	Height FileBlockStorage::chainHeight() const {
		return m_indexFile.exists() ? Height(m_indexFile.get()) : Height(0);
	}

	model::HashRange FileBlockStorage::loadHashesFrom(Height height, size_t maxHashes) const {
		if (FileBlockStorageMode::Hash_Index != m_mode)
			CATAPULT_THROW_INVALID_ARGUMENT("loadHashesFrom is not supported when Hash_Index mode is disabled");

		auto currentHeight = chainHeight();
		if (Height(0) == height || currentHeight < height)
			return model::HashRange();

		auto numAvailableHashes = static_cast<size_t>((currentHeight - height).unwrap() + 1);
		auto numHashes = std::min(maxHashes, numAvailableHashes);
		return m_hashFile.loadRangeFrom(height, numHashes);
	}

	void FileBlockStorage::saveBlock(const model::BlockElement& blockElement) {
		auto currentHeight = chainHeight();
		auto height = blockElement.Block.Height;

		if (height != currentHeight + Height(1)) {
			std::ostringstream out;
			out << "cannot save block with height " << height << " when storage height is " << currentHeight;
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		{
			// write element
			auto pBlockStream = m_blockDatabase.outputStream(height.unwrap());
			WriteBlockElement(blockElement, *pBlockStream);

			// write statements
			if (blockElement.OptionalStatement) {
				auto pBlockStatementStream = m_statementDatabase.outputStream(height.unwrap());
				WriteBlockStatement(*blockElement.OptionalStatement, *pBlockStatementStream);
			}
		}

		if (FileBlockStorageMode::Hash_Index == m_mode)
			m_hashFile.save(height, blockElement.EntityHash);

		if (height > currentHeight)
			m_indexFile.set(height.unwrap());
	}

	void FileBlockStorage::dropBlocksAfter(Height height) {
		m_indexFile.set(height.unwrap());
	}

	// endregion

	// region BlockStorage

	namespace {
		std::shared_ptr<model::Block> ReadBlock(InputStream& blockStream) {
			auto size = Read32(blockStream);
			auto pBlock = utils::MakeSharedWithSize<model::Block>(size);
			pBlock->Size = size;
			blockStream.read({ reinterpret_cast<uint8_t*>(pBlock.get()) + sizeof(uint32_t), size - sizeof(uint32_t) });
			return pBlock;
		}
	}

	std::shared_ptr<const model::Block> FileBlockStorage::loadBlock(Height height) const {
		requireHeight(height, "block");
		auto pBlockStream = m_blockDatabase.inputStream(height.unwrap());
		return ReadBlock(*pBlockStream);
	}

	std::shared_ptr<const model::BlockElement> FileBlockStorage::loadBlockElement(Height height) const {
		requireHeight(height, "block element");
		auto pBlockStream = m_blockDatabase.inputStream(height.unwrap());
		auto pBlockElement = ReadBlockElement(*pBlockStream);

		if (!pBlockStream->eof())
			CATAPULT_THROW_RUNTIME_ERROR_1("additional data after block at height", height);

		return PORTABLE_MOVE(pBlockElement);
	}

	std::pair<std::vector<uint8_t>, bool> FileBlockStorage::loadBlockStatementData(Height height) const {
		requireHeight(height, "block statement data");

		if (!m_statementDatabase.contains(height.unwrap()))
			return std::make_pair(std::vector<uint8_t>(), false);

		size_t streamSize = 0;
		auto pBlockStatementStream = m_statementDatabase.inputStream(height.unwrap(), &streamSize);

		std::vector<uint8_t> blockStatement;
		blockStatement.resize(streamSize);
		pBlockStatementStream->read(blockStatement);

		return std::make_pair(std::move(blockStatement), true);
	}

	// endregion

	// region PrunableBlockStorage

	void FileBlockStorage::purge() {
		// remove everything under the directory
		m_hashFile.reset();
		PurgeDirectory(m_dataDirectory);
	}

	// endregion

	// region requireHeight

	void FileBlockStorage::requireHeight(Height height, const char* description) const {
		auto chainHeight = this->chainHeight();
		if (height <= chainHeight)
			return;

		std::ostringstream out;
		out << "cannot load " << description << " at height (" << height << ") greater than chain height (" << chainHeight << ")";
		CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
	}

	// endregion
}}
