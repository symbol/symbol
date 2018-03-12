#pragma once
#include "BlockStorage.h"
#include "catapult/utils/SpinReaderWriterLock.h"

namespace catapult { namespace io { struct CachedData; } }

namespace catapult { namespace io {

	/// A read only view on top of block storage.
	class BlockStorageView : utils::MoveOnly {
	public:
		/// Creates a view around \a storage and cache data (\a cachedData) with lock context \a readLock.
		explicit BlockStorageView(
				const BlockStorage& storage,
				utils::SpinReaderWriterLock::ReaderLockGuard&& readLock,
				const CachedData& cachedData)
				: m_storage(storage)
				, m_readLock(std::move(readLock))
				, m_cachedData(cachedData)
		{}

	public:
		/// Gets the number of blocks.
		Height chainHeight() const;

		/// Returns the block at \a height.
		std::shared_ptr<const model::Block> loadBlock(Height height) const;

		/// Returns the block element (owning a block) at \a height.
		std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const;

		/// Returns a range of at most \a maxHashes hashes starting at \a height.
		model::HashRange loadHashesFrom(Height height, size_t maxHashes) const;

	private:
		const BlockStorage& m_storage;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
		const CachedData& m_cachedData;
	};

	/// A write only view on top of block storage.
	class BlockStorageModifier : utils::MoveOnly {
	public:
		/// Creates a view around \a storage and cache data (\a cachedData) with lock context \a readLock.
		explicit BlockStorageModifier(
				BlockStorage& storage,
				utils::SpinReaderWriterLock::ReaderLockGuard&& readLock,
				CachedData& cachedData)
				: m_storage(storage)
				, m_readLock(std::move(readLock))
				, m_writeLock(m_readLock.promoteToWriter())
				, m_cachedData(cachedData)
		{}

	public:
		/// Saves a block element (\a blockElement).
		void saveBlock(const model::BlockElement& blockElement);

		/// Saves multiple block elements (\a blockElements).
		void saveBlocks(const std::vector<model::BlockElement>& blockElements);

		/// Drops all blocks after \a height.
		void dropBlocksAfter(Height height);

	private:
		BlockStorage& m_storage;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
		utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
		CachedData& m_cachedData;
	};

	/// A cache around a BlockStorage.
	/// \note Currently this "cache" only provides synchronization.
	class BlockStorageCache {
	public:
		/// Creates a new cache around \a pStorage.
		explicit BlockStorageCache(std::unique_ptr<BlockStorage>&& pStorage);

		/// Destroys the cache.
		~BlockStorageCache();

	public:
		/// Gets a read only view of the storage.
		BlockStorageView view() const;

		/// Gets a write only view of the storage.
		BlockStorageModifier modifier();

	private:
		std::unique_ptr<BlockStorage> m_pStorage;
		std::unique_ptr<CachedData> m_pCachedData;
		mutable utils::SpinReaderWriterLock m_lock;
	};
}}
