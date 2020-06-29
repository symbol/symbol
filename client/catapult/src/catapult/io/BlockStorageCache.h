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
#include "catapult/utils/SpinReaderWriterLock.h"

namespace catapult { namespace io { struct CachedData; } }

namespace catapult { namespace io {

	/// Read only view on top of block storage.
	class BlockStorageView : utils::MoveOnly {
	public:
		/// Creates a view around \a storage and cache data (\a cachedData) with lock context \a readLock.
		BlockStorageView(
				const BlockStorage& storage,
				utils::SpinReaderWriterLock::ReaderLockGuard&& readLock,
				const CachedData& cachedData);

	public:
		/// Gets the number of blocks.
		Height chainHeight() const;

		/// Gets a range of at most \a maxHashes hashes starting at \a height.
		model::HashRange loadHashesFrom(Height height, size_t maxHashes) const;

		/// Gets the block at \a height.
		std::shared_ptr<const model::Block> loadBlock(Height height) const;

		/// Gets the block element (owning a block) at \a height.
		std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const;

		/// Gets the optional block statement data at \a height.
		std::pair<std::vector<uint8_t>, bool> loadBlockStatementData(Height height) const;

	private:
		void requireHeight(Height height, const char* description) const;

	private:
		const BlockStorage& m_storage;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
		const CachedData& m_cachedData;
	};

	/// Write only view on top of block storage.
	class BlockStorageModifier : utils::MoveOnly {
	public:
		/// Creates a view around \a storage, \a stagingStorage and cache data (\a cachedData) with lock context \a writeLock.
		BlockStorageModifier(
				BlockStorage& storage,
				PrunableBlockStorage& stagingStorage,
				utils::SpinReaderWriterLock::WriterLockGuard&& writeLock,
				CachedData& cachedData);

	public:
		/// Saves a block element (\a blockElement).
		void saveBlock(const model::BlockElement& blockElement);

		/// Saves multiple block elements (\a blockElements).
		void saveBlocks(const std::vector<model::BlockElement>& blockElements);

		/// Drops all blocks after \a height.
		void dropBlocksAfter(Height height);

		/// Commits all staged changes to the primary storage.
		void commit();

	private:
		BlockStorage& m_storage;
		PrunableBlockStorage& m_stagingStorage;
		utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
		CachedData& m_cachedData;
		Height m_saveStartHeight;
	};

	/// Cache around a BlockStorage.
	/// \note Currently this "cache" provides synchronization and support for two-phase commit.
	class BlockStorageCache {
	public:
		/// Creates a new cache around \a pStorage that uses \a pStagingStorage for staging blocks in order to enable two-phase commit.
		BlockStorageCache(std::unique_ptr<BlockStorage>&& pStorage, std::unique_ptr<PrunableBlockStorage>&& pStagingStorage);

		/// Destroys the cache.
		~BlockStorageCache();

	public:
		/// Gets a read only view of the storage.
		BlockStorageView view() const;

		/// Gets a write only view of the storage.
		BlockStorageModifier modifier();

	private:
		std::unique_ptr<BlockStorage> m_pStorage;
		std::unique_ptr<PrunableBlockStorage> m_pStagingStorage;
		std::unique_ptr<CachedData> m_pCachedData;
		mutable utils::SpinReaderWriterLock m_lock;
	};
}}
