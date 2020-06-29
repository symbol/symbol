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

	namespace {
		static constexpr auto Block_File_Extension = ".dat";
		static constexpr auto Block_Statement_File_Extension = ".stmt";

		// region path utils

		auto GetStorageDirectory(const std::string& baseDirectory, Height height) {
			return config::CatapultStorageDirectoryPreparer::Prepare(baseDirectory, height);
		}

		std::string GetBlockStatementPath(const std::string& baseDirectory, Height height) {
			auto storageDir = GetStorageDirectory(baseDirectory, height);
			return storageDir.storageFile(Block_Statement_File_Extension);
		}

		// endregion

		// region file utils

		bool IsRegularFile(const boost::filesystem::path& path) {
			return boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path);
		}

		auto OpenBlockFile(const std::string& baseDirectory, Height height, OpenMode mode = OpenMode::Read_Only) {
			auto storageDir = GetStorageDirectory(baseDirectory, height);
			auto blockPath = storageDir.storageFile(Block_File_Extension);
			return std::make_unique<RawFile>(blockPath, mode);
		}

		auto OpenBlockStatementFile(const std::string& baseDirectory, Height height, OpenMode mode = OpenMode::Read_Only) {
			auto blockStatementPath = GetBlockStatementPath(baseDirectory, height);
			return RawFile(blockStatementPath, mode);
		}

		// endregion
	}

	// region ctor

	FileBlockStorage::FileBlockStorage(const std::string& dataDirectory, FileBlockStorageMode mode)
			: m_dataDirectory(dataDirectory)
			, m_mode(mode)
			, m_hashFile(m_dataDirectory, "hashes")
			, m_indexFile((boost::filesystem::path(m_dataDirectory) / "index.dat").generic_string())
	{}

	// endregion

	// region LightBlockStorage

	namespace {
		// use RawFile adapter instead of BufferedFileStream because everything read/written is in consecutive memory,
		// so there's no benefit to buffering
		class RawFileOutputStreamAdapter : public OutputStream {
		public:
			explicit RawFileOutputStreamAdapter(RawFile& rawFile) : m_rawFile(rawFile)
			{}

		public:
			void write(const RawBuffer& buffer) override {
				m_rawFile.write(buffer);
			}

			void flush() override {
				CATAPULT_THROW_INVALID_ARGUMENT("flush not supported");
			}

		private:
			RawFile& m_rawFile;
		};
	}

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
			auto pBlockFile = OpenBlockFile(m_dataDirectory, height, OpenMode::Read_Write);
			RawFileOutputStreamAdapter streamAdapter(*pBlockFile);
			WriteBlockElement(blockElement, streamAdapter);

			// write statements
			if (blockElement.OptionalStatement) {
				BufferedOutputFileStream blockStatementOutputStream(OpenBlockStatementFile(m_dataDirectory, height, OpenMode::Read_Write));
				WriteBlockStatement(*blockElement.OptionalStatement, blockStatementOutputStream);
				blockStatementOutputStream.flush();
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
		class RawFileInputStreamAdapter : public InputStream {
		public:
			explicit RawFileInputStreamAdapter(RawFile& rawFile) : m_rawFile(rawFile)
			{}

		public:
			bool eof() const override {
				CATAPULT_THROW_INVALID_ARGUMENT("eof not supported");
			}

			void read(const MutableRawBuffer& buffer) override {
				m_rawFile.read(buffer);
			}

		private:
			RawFile& m_rawFile;
		};

		std::shared_ptr<model::Block> ReadBlock(RawFile& blockFile) {
			auto size = Read32(blockFile);
			blockFile.seek(0);

			auto pBlock = utils::MakeSharedWithSize<model::Block>(size);
			blockFile.read({ reinterpret_cast<uint8_t*>(pBlock.get()), size });
			return pBlock;
		}
	}

	std::shared_ptr<const model::Block> FileBlockStorage::loadBlock(Height height) const {
		requireHeight(height, "block");
		auto pBlockFile = OpenBlockFile(m_dataDirectory, height);
		return ReadBlock(*pBlockFile);
	}

	std::shared_ptr<const model::BlockElement> FileBlockStorage::loadBlockElement(Height height) const {
		requireHeight(height, "block element");
		auto pBlockFile = OpenBlockFile(m_dataDirectory, height);
		RawFileInputStreamAdapter streamAdapter(*pBlockFile);
		auto pBlockElement = ReadBlockElement(streamAdapter);

		if (pBlockFile->position() != pBlockFile->size())
			CATAPULT_THROW_RUNTIME_ERROR_1("additional data after block at height", height);

		return PORTABLE_MOVE(pBlockElement);
	}

	std::pair<std::vector<uint8_t>, bool> FileBlockStorage::loadBlockStatementData(Height height) const {
		requireHeight(height, "block statement data");
		auto path = GetBlockStatementPath(m_dataDirectory, height);
		if (!IsRegularFile(path))
			return std::make_pair(std::vector<uint8_t>(), false);

		auto blockStatementFile = OpenBlockStatementFile(m_dataDirectory, height);
		std::vector<uint8_t> blockStatement;
		blockStatement.resize(blockStatementFile.size());
		blockStatementFile.read(blockStatement);
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
