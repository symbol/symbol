#pragma once
#include "HashCacheTypes.h"
#include "catapult/cache/ReadOnlySimpleCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/utils/SpinReaderWriterLock.h"
#include "catapult/utils/TimeSpan.h"

namespace catapult { namespace cache {

	/// Basic view on top of the hash cache.
	class BasicHashCacheView : public utils::MoveOnly {
	public:
		using ReadOnlyView = hash_cache_types::CacheReadOnlyType;
		using ValueType = hash_cache_types::ValueType;

	public:
		/// Creates a view with the specified retention time (\a retentionTime) around \a orderedSet.
		explicit BasicHashCacheView(const utils::TimeSpan& retentionTime, const hash_cache_types::BaseSetType& orderedSet)
				: m_retentionTime(retentionTime)
				, m_orderedSet(orderedSet)
		{}

	public:
		/// Gets the retention time for the cache.
		utils::TimeSpan retentionTime() const {
			return m_retentionTime;
		}

		/// Gets the size of the cache.
		size_t size() const {
			return m_orderedSet.size();
		}

		/// Searches for the given timestamped hash (\a timestampedHash) in the cache.
		/// Returns \c true if it is found or \c false if it is not found.
		bool contains(const ValueType& timestampedHash) const {
			return m_orderedSet.contains(timestampedHash);
		}

	public:
		/// Returns a const iterator to the first element of the underlying set.
		auto cbegin() const {
			return m_orderedSet.cbegin();
		}

		/// Returns a const iterator to the element following the last element of the underlying set.
		auto cend() const {
			return m_orderedSet.cend();
		}

	private:
		utils::TimeSpan m_retentionTime;
		const hash_cache_types::BaseSetType& m_orderedSet;
	};

	/// View on top of the hash cache.
	class HashCacheView : public ReadOnlyViewSupplier<BasicHashCacheView> {
	public:
		/// Creates a view with the specified retention time (\a retentionTime) around \a orderedSet.
		explicit HashCacheView(const utils::TimeSpan& retentionTime, const hash_cache_types::BaseSetType& orderedSet)
				: ReadOnlyViewSupplier(retentionTime, orderedSet)
		{}
	};
}}
