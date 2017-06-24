#pragma once
#include "catapult/utils/SpinReaderWriterLock.h"
#include "catapult/types.h"

namespace catapult { namespace cache {

	/// A read only view on top of a cache height.
	class CacheHeightView : public utils::MoveOnly {
	public:
		/// Creates a cache height view around \a height with lock context \a readLock.
		explicit CacheHeightView(
				Height height,
				utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
				: m_height(height)
				, m_readLock(std::move(readLock))
		{}

	public:
		/// Returns the height.
		Height get() const {
			return m_height;
		}

	private:
		Height m_height;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
	};

	/// A write only view on top of a cache height.
	class CacheHeightModifier : public utils::MoveOnly {
	public:
		/// Creates a write only view around \a height with lock context \a readLock.
		explicit CacheHeightModifier(
				Height& height,
				utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
				: m_height(height)
				, m_readLock(std::move(readLock))
				, m_writeLock(m_readLock.promoteToWriter())
		{}

	public:
		/// Sets the cache height to \a height.
		void set(Height height) {
			m_height = height;
		}

	private:
		Height& m_height;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
		utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
	};

	/// A synchronized height associated with a catapult cache.
	class CacheHeight {
	public:
		/// Gets a read only view of the height.
		CacheHeightView view() const {
			return CacheHeightView(m_height, m_lock.acquireReader());
		}

		/// Gets a write only view of the height.
		CacheHeightModifier modifier() {
			return CacheHeightModifier(m_height, m_lock.acquireReader());
		}

	private:
		Height m_height;
		mutable utils::SpinReaderWriterLock m_lock;
	};
}}
