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

#pragma once
#include "catapult/utils/NonCopyable.h"
#include "catapult/utils/SpinReaderWriterLock.h"
#include <boost/optional.hpp>

namespace catapult { namespace cache {

	namespace detail {
		// region CacheViewReadLockPair

		/// Cache-view, read-lock pair.
		/// \note The cache-view template argument can be either a view or a delta.
		template<typename TCacheView>
		struct CacheViewReadLockPair {
		public:
			/// Creates a pair around \a cacheView and \a readLock.
			CacheViewReadLockPair(TCacheView&& cacheView, utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
					: CacheView(std::move(cacheView))
					, ReadLock(std::move(readLock))
			{}

		public:
			/// Cache view.
			TCacheView CacheView;

			/// Read lock.
			utils::SpinReaderWriterLock::ReaderLockGuard ReadLock;
		};

		// endregion

		// region CacheViewAccessor

		/// Cache view accessor.
		template<typename TCacheView>
		class CacheViewAccessor {
		public:
			/// Creates an accessor around \a pCacheView;
			explicit CacheViewAccessor(TCacheView* pCacheView) : m_pCacheView(pCacheView)
			{}

		public:
			/// Returns \c true if the underlying cache view is valid.
			explicit operator bool() const {
				return !!m_pCacheView;
			}

			/// Gets a const pointer to the underlying cache view.
			const TCacheView* operator->() const {
				return m_pCacheView;
			}

			/// Gets a const reference to the underlying cache view.
			const TCacheView& operator*() const {
				return *m_pCacheView;
			}

		public:
			/// Gets a pointer to the underlying cache view.
			TCacheView* operator->() {
				return m_pCacheView;
			}

			/// Gets a reference to the underlying cache view.
			TCacheView& operator*() {
				return *m_pCacheView;
			}

		private:
			TCacheView* m_pCacheView;
		};

		// endregion
	}

	// region LockedCacheView

	/// Cache-view, read-lock pair.
	/// \note This is similar to but provides a better interface than detail::CacheViewReadLockPair.
	template<typename TCacheView>
	class LockedCacheView : public detail::CacheViewAccessor<TCacheView> {
	public:
		/// Creates a view around \a cacheView and \a readLock.
		LockedCacheView(TCacheView&& cacheView, utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
				: detail::CacheViewAccessor<TCacheView>(&m_cacheView)
				, m_cacheView(std::move(cacheView))
				, m_readLock(std::move(readLock))
		{}

		/// Move constructs a view from \a rhs.
		/// \note Default move constructor will not work because pointer in CacheViewAccessor needs to be updated.
		LockedCacheView(LockedCacheView&& rhs)
				: detail::CacheViewAccessor<TCacheView>(&m_cacheView)
				, m_cacheView(std::move(rhs.m_cacheView))
				, m_readLock(std::move(rhs.m_readLock))
		{}

	private:
		TCacheView m_cacheView;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
	};

	// endregion

	// region LockedCacheDelta

	/// Cache-delta, read-lock pair.
	template<typename TCacheView>
	class LockedCacheDelta : public detail::CacheViewAccessor<TCacheView> {
	public:
		/// Creates a view around \a pPair.
		explicit LockedCacheDelta(const std::shared_ptr<detail::CacheViewReadLockPair<TCacheView>>& pPair)
				: detail::CacheViewAccessor<TCacheView>(&pPair->CacheView)
				, m_pPair(pPair)
		{}

	private:
		std::shared_ptr<detail::CacheViewReadLockPair<TCacheView>> m_pPair;
	};

	// endregion

	// region OptionalLockedCacheDelta

	/// Optional cache-delta, read-lock pair.
	template<typename TCacheView>
	class OptionalLockedCacheDelta : public detail::CacheViewAccessor<TCacheView> {
	public:
		/// Creates a empty view.
		OptionalLockedCacheDelta() : detail::CacheViewAccessor<TCacheView>(nullptr)
		{}

		/// Creates a view around \a cacheView and \a pReadLock.
		OptionalLockedCacheDelta(TCacheView& cacheView, utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
				: detail::CacheViewAccessor<TCacheView>(&cacheView)
				, m_readLock(std::move(readLock))
		{}

	private:
		boost::optional<utils::SpinReaderWriterLock::ReaderLockGuard> m_readLock;
	};

	// endregion

	// region LockableCacheDelta

	template<typename TCacheDelta>
	class LockableCacheDelta {
	public:
		/// Creates a lockable cache delta around \a cacheDelta using the specified \a lock
		/// and commit counter (\a commitCounter).
		LockableCacheDelta(TCacheDelta&& cacheDelta, const size_t& commitCounter, utils::SpinReaderWriterLock& lock)
				: m_cacheDelta(std::move(cacheDelta))
				, m_initialCommitCount(commitCounter)
				, m_commitCounter(commitCounter)
				, m_lock(lock)
		{}

	public:
		/// Locks the cache delta.
		/// \note Returns a falsy structure if the lockable delta is no longer valid.
		OptionalLockedCacheDelta<TCacheDelta> tryLock() {
			auto readLock = m_lock.acquireReader();
			return m_initialCommitCount != m_commitCounter
					? OptionalLockedCacheDelta<TCacheDelta>()
					: OptionalLockedCacheDelta<TCacheDelta>(m_cacheDelta, std::move(readLock));
		}

	private:
		TCacheDelta m_cacheDelta;
		size_t m_initialCommitCount;
		const size_t& m_commitCounter;
		utils::SpinReaderWriterLock& m_lock;
	};

	// endregion

	// region SynchronizedCache

	/// Decorator that synchronizes access to a cache.
	template<typename TCache>
	class SynchronizedCache {
	public:
		using CacheValueType = typename TCache::CacheValueType;
		using CacheViewType = typename TCache::CacheViewType;
		using CacheDeltaType = typename TCache::CacheDeltaType;
		using CacheReadOnlyType = typename TCache::CacheReadOnlyType;

	public:
		/// Creates a synchronized decorator around \a cache.
		explicit SynchronizedCache(TCache&& cache)
				: m_cache(std::move(cache))
				, m_commitCounter(0)
		{}

	public:
		/// Gets a locked cache view based on this cache.
		LockedCacheView<CacheViewType> createView() const {
			auto readLock = m_lock.acquireReader();
			return LockedCacheView<CacheViewType>(m_cache.createView(), std::move(readLock));
		}

		/// Gets a locked cache delta based on this cache.
		/// \note Changes to an attached delta can be committed by calling commit.
		LockedCacheDelta<CacheDeltaType> createDelta() {
			auto readLock = m_lock.acquireReader();

			// notice that this is not a foolproof check since multiple threads could create multiple deltas at the same time
			// but it is good enough as a sanity check
			if (m_pWeakDeltaPair.lock())
				CATAPULT_THROW_RUNTIME_ERROR("only a single attached delta of a cache is allowed at a time");

			auto delta = m_cache.createDelta();
			auto pPair = std::make_shared<detail::CacheViewReadLockPair<CacheDeltaType>>(std::move(delta), std::move(readLock));
			m_pWeakDeltaPair = pPair;
			return LockedCacheDelta<CacheDeltaType>(pPair);
		}

		/// Gets a lockable cache delta based on this cache but without the ability
		/// to commit any changes to the original cache.
		LockableCacheDelta<CacheDeltaType> createDetachedDelta() const {
			auto readLock = m_lock.acquireReader();
			auto delta = m_cache.createDetachedDelta();
			return LockableCacheDelta<CacheDeltaType>(std::move(delta), m_commitCounter, m_lock);
		}

		/// Commits all pending changes to the underlying storage.
		void commit() {
			auto pDeltaPair = m_pWeakDeltaPair.lock();
			if (!pDeltaPair)
				CATAPULT_THROW_RUNTIME_ERROR("attempting to commit changes to a cache without any outstanding attached deltas");

			auto writeLock = pDeltaPair->ReadLock.promoteToWriter();
			m_cache.commit(pDeltaPair->CacheView);
			++m_commitCounter;
		}

	protected:
		/// Gets a typed reference to the underlying cache.
		TCache& cache() {
			return m_cache;
		}

	private:
		TCache m_cache;
		size_t m_commitCounter;
		std::weak_ptr<detail::CacheViewReadLockPair<CacheDeltaType>> m_pWeakDeltaPair;
		mutable utils::SpinReaderWriterLock m_lock;
	};

	// endregion

	// region SynchronizedCacheWithInit

	/// Decorator that synchronizes access to a cache and allows custom cache initialization.
	template<typename TCache>
	class SynchronizedCacheWithInit : public SynchronizedCache<TCache> {
	public:
		/// Creates a synchronized decorator around \a cache.
		explicit SynchronizedCacheWithInit(TCache&& cache)
				: SynchronizedCache<TCache>(std::move(cache))
				, m_isInitCalled(false)
		{}

	public:
		/// Initializes the underlying cache with \a args.
		template<typename... TArgs>
		void init(TArgs&&... args) {
			if (m_isInitCalled)
				CATAPULT_THROW_RUNTIME_ERROR("init has already been called for cache");

			this->cache().init(std::forward<TArgs>(args)...);
			m_isInitCalled = true;
		}

	private:
		bool m_isInitCalled;
	};

	// endregion
}}
