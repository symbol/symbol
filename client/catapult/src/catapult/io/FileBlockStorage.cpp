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
#include "BlockStatementSerializer.h"
#include "BufferedFileStream.h"
#include "PodIoUtils.h"
#include "catapult/utils/MemoryUtils.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <inttypes.h>

namespace catapult { namespace io {

	namespace {
		static constexpr uint64_t Unset_Directory_Id = std::numeric_limits<uint64_t>::max();
		static constexpr uint32_t Files_Per_Directory = 65536u;
		static constexpr auto Block_File_Extension = ".dat";
		static constexpr auto Block_Statement_File_Extension = ".stmt";
		static constexpr auto Index_File = "index.dat";

		// region path utils

#ifdef _MSC_VER
#define SPRINTF sprintf_s
#else
#define SPRINTF sprintf
#endif

		boost::filesystem::path GetDirectoryPath(const std::string& baseDirectory, Height height) {
			char subDirectory[16];
			SPRINTF(subDirectory, "%05" PRId64, height.unwrap() / Files_Per_Directory);
			boost::filesystem::path path = baseDirectory;
			path /= subDirectory;
			if (!boost::filesystem::exists(path))
				boost::filesystem::create_directory(path);

			return path;
		}

		boost::filesystem::path GetBlockPath(const std::string& baseDirectory, Height height, const char* extension) {
			auto path = GetDirectoryPath(baseDirectory, height);
			char filename[16];
			SPRINTF(filename, "%05" PRId64, height.unwrap() % Files_Per_Directory);
			path /= filename;
			path += extension;
			return path;
		}

		boost::filesystem::path GetHashFilePath(const std::string& baseDirectory, Height height) {
			auto path = GetDirectoryPath(baseDirectory, height);
			path /= "hashes.dat";
			return path;
		}

		boost::filesystem::path GetBlockStatementPath(const std::string& baseDirectory, Height height) {
			return GetBlockPath(baseDirectory, height, Block_Statement_File_Extension);
		}

		// endregion

		// region file utils

		bool IsRegularFile(const boost::filesystem::path& path) {
			return boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path);
		}

		bool HasJournal(const std::string& baseDirectory) {
			boost::filesystem::path journalPath = baseDirectory;
			journalPath /= Index_File;
			return IsRegularFile(journalPath);
		}

		auto OpenBlockFile(const std::string& baseDirectory, Height height, OpenMode mode = OpenMode::Read_Only) {
			auto blockPath = GetBlockPath(baseDirectory, height, Block_File_Extension);
			return std::make_unique<RawFile>(blockPath.generic_string().c_str(), mode);
		}

		auto OpenBlockStatementFile(const std::string& baseDirectory, Height height, OpenMode mode = OpenMode::Read_Only) {
			auto blockStatementPath = GetBlockStatementPath(baseDirectory, height);
			return RawFile(blockStatementPath.generic_string().c_str(), mode);
		}

		auto OpenJournalFile(const std::string& baseDirectory, OpenMode mode = OpenMode::Read_Only) {
			boost::filesystem::path journalPath = baseDirectory;
			journalPath /= Index_File;
			return std::make_unique<RawFile>(journalPath.generic_string().c_str(), mode);
		}

		// endregion
	}

	// region FileBlockStorage::HashFile

	FileBlockStorage::HashFile::HashFile(const std::string& dataDirectory)
			: m_dataDirectory(dataDirectory)
			, m_cachedDirectoryId(Unset_Directory_Id)
	{}

	namespace {
		std::unique_ptr<RawFile> OpenHashFile(const std::string& baseDirectory, Height height, OpenMode openMode) {
			auto hashFilePath = GetHashFilePath(baseDirectory, height);
			auto pHashFile = std::make_unique<RawFile>(hashFilePath.generic_string().c_str(), openMode, LockMode::None);
			// check that first hash file has at least two hashes inside.
			if (height.unwrap() < Files_Per_Directory && Hash256_Size * 2 > pHashFile->size())
				CATAPULT_THROW_RUNTIME_ERROR_1("hashes.dat has invalid size", pHashFile->size());

			return pHashFile;
		}

		void SeekHashFile(RawFile& hashFile, Height height) {
			auto index = height.unwrap() % Files_Per_Directory;
			hashFile.seek(index * Hash256_Size);
		}
	}

	model::HashRange FileBlockStorage::HashFile::loadHashesFrom(Height height, size_t numHashes) const {
		uint8_t* pData = nullptr;
		auto range = model::HashRange::PrepareFixed(numHashes, &pData);

		while (numHashes) {
			auto pHashFile = OpenHashFile(m_dataDirectory, height, OpenMode::Read_Only);
			SeekHashFile(*pHashFile, height);

			auto count = Files_Per_Directory - (height.unwrap() % Files_Per_Directory);
			count = std::min<size_t>(numHashes, count);

			pHashFile->read(MutableRawBuffer(pData, count * Hash256_Size));

			pData += count * Hash256_Size;
			numHashes -= count;
			height = height + Height(count);
		}

		return range;
	}

	void FileBlockStorage::HashFile::save(Height height, const Hash256& hash) {
		auto currentId = height.unwrap() / Files_Per_Directory;
		if (m_cachedDirectoryId != currentId) {
			m_pCachedHashFile = OpenHashFile(m_dataDirectory, height, OpenMode::Read_Append);
			m_cachedDirectoryId = currentId;
		}

		SeekHashFile(*m_pCachedHashFile, height);
		m_pCachedHashFile->write(hash);
	}

	// endregion

	// region ctor

	FileBlockStorage::FileBlockStorage(const std::string& dataDirectory)
			: m_dataDirectory(dataDirectory)
			, m_hashFile(m_dataDirectory)
	{}

	// endregion

	// region LightBlockStorage

	namespace {
		void SetHeight(const std::string& baseDirectory, Height height) {
			auto pJournalFile = OpenJournalFile(baseDirectory, OpenMode::Read_Write);
			Write(*pJournalFile, height);
		}

		void WriteTransactionHashes(RawFile& blockFile, const std::vector<model::TransactionElement>& transactionElements) {
			auto numTransactions = static_cast<uint32_t>(transactionElements.size());
			Write32(blockFile, numTransactions);
			std::vector<Hash256> hashes(2 * numTransactions);
			auto iter = hashes.begin();
			for (const auto& transactionElement : transactionElements) {
				*iter++ = transactionElement.EntityHash;
				*iter++ = transactionElement.MerkleComponentHash;
			}

			blockFile.write({ reinterpret_cast<const uint8_t*>(hashes.data()), hashes.size() * Hash256_Size });
		}

		void WriteSubCacheMerkleRoots(RawFile& blockFile, const std::vector<Hash256>& subCacheMerkleRoots) {
			auto numHashes = static_cast<uint32_t>(subCacheMerkleRoots.size());
			Write32(blockFile, numHashes);
			blockFile.write({ reinterpret_cast<const uint8_t*>(subCacheMerkleRoots.data()), numHashes * Hash256_Size });
		}
	}

	Height FileBlockStorage::chainHeight() const {
		if (!HasJournal(m_dataDirectory))
			return Height(1);

		auto pJournalFile = OpenJournalFile(m_dataDirectory);
		return Read<Height>(*pJournalFile);
	}

	model::HashRange FileBlockStorage::loadHashesFrom(Height height, size_t maxHashes) const {
		auto currentHeight = chainHeight();
		if (Height(0) == height || currentHeight < height)
			return model::HashRange();

		auto numAvailableHashes = static_cast<size_t>((currentHeight - height).unwrap() + 1);
		auto numHashes = std::min(maxHashes, numAvailableHashes);
		return m_hashFile.loadHashesFrom(height, numHashes);
	}

	void FileBlockStorage::saveBlock(const model::BlockElement& blockElement) {
		auto currentHeight = chainHeight();
		auto height = blockElement.Block.Height;

		if (height != currentHeight + Height(1))
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot save out of order block at height", height);

		{
			// 1. write constant size data
			auto pBlockFile = OpenBlockFile(m_dataDirectory, height, OpenMode::Read_Write);
			pBlockFile->write({ reinterpret_cast<const uint8_t*>(&blockElement.Block), blockElement.Block.Size });
			pBlockFile->write(blockElement.EntityHash);
			pBlockFile->write(blockElement.GenerationHash);

			// 2. write transaction hashes
			WriteTransactionHashes(*pBlockFile, blockElement.Transactions);

			// 3. write sub cache merkle roots
			WriteSubCacheMerkleRoots(*pBlockFile, blockElement.SubCacheMerkleRoots);

			// 4. write statements
			Write32(*pBlockFile, blockElement.OptionalStatement ? 0xFFFF'FFFF : 0);
			if (blockElement.OptionalStatement) {
				BufferedOutputFileStream blockStatementOutputStream(OpenBlockStatementFile(m_dataDirectory, height, OpenMode::Read_Write));
				WriteBlockStatement(blockStatementOutputStream, *blockElement.OptionalStatement);
				blockStatementOutputStream.flush();
			}
		}

		m_hashFile.save(height, blockElement.EntityHash);

		if (height > currentHeight)
			SetHeight(m_dataDirectory, height);
	}

	void FileBlockStorage::dropBlocksAfter(Height height) {
		SetHeight(m_dataDirectory, height);
	}

	// endregion

	// region BlockStorage

	namespace {
		std::shared_ptr<model::Block> ReadBlock(RawFile& blockFile) {
			auto size = Read32(blockFile);
			blockFile.seek(0);

			auto pBlock = utils::MakeSharedWithSize<model::Block>(size);
			blockFile.read({ reinterpret_cast<uint8_t*>(pBlock.get()), size });
			return pBlock;
		}

		std::shared_ptr<model::BlockElement> ReadBlockElement(RawFile& blockFile) {
			auto size = Read32(blockFile);
			blockFile.seek(0);

			// allocate memory for both the element and the block in one shot (Block data is appended)
			auto pData = utils::MakeUniqueWithSize<uint8_t>(sizeof(model::BlockElement) + size);

			// read the block data
			auto pBlockData = pData.get() + sizeof(model::BlockElement);
			blockFile.read({ pBlockData, size });

			// create the block element and transfer ownership from pData to pBlockElement
			auto pBlockElementRaw = new (pData.get()) model::BlockElement(*reinterpret_cast<model::Block*>(pBlockData));
			auto pBlockElement = std::shared_ptr<model::BlockElement>(pBlockElementRaw);
			pData.release();

			// read metadata
			blockFile.read(pBlockElement->EntityHash);
			blockFile.read(pBlockElement->GenerationHash);
			return pBlockElement;
		}

		void ReadTransactionHashes(RawFile& blockFile, model::BlockElement& blockElement) {
			auto numTransactions = Read32(blockFile);
			std::vector<Hash256> hashes(2 * numTransactions);
			blockFile.read({ reinterpret_cast<uint8_t*>(hashes.data()), hashes.size() * Hash256_Size });

			size_t i = 0;
			for (const auto& transaction : blockElement.Block.Transactions()) {
				blockElement.Transactions.push_back(model::TransactionElement(transaction));
				blockElement.Transactions.back().EntityHash = hashes[i++];
				blockElement.Transactions.back().MerkleComponentHash = hashes[i++];
			}
		}

		void ReadSubCacheMerkleRoots(RawFile& blockFile, std::vector<Hash256>& subCacheMerkleRoots) {
			auto numHashes = Read32(blockFile);
			subCacheMerkleRoots.resize(numHashes);
			blockFile.read({ reinterpret_cast<uint8_t*>(subCacheMerkleRoots.data()), numHashes * Hash256_Size });
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
		auto pBlockElement = ReadBlockElement(*pBlockFile);

		ReadTransactionHashes(*pBlockFile, *pBlockElement);
		ReadSubCacheMerkleRoots(*pBlockFile, pBlockElement->SubCacheMerkleRoots);

		auto optionalStatementMarker = Read32(*pBlockFile);
		if (0 == optionalStatementMarker)
			return std::move(pBlockElement);

		if (0xFFFF'FFFF != optionalStatementMarker)
			CATAPULT_THROW_RUNTIME_ERROR_1("block file has invalid marker, height", pBlockElement->Block.Height);

		return std::move(pBlockElement);
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
