#pragma once
#include "HashCacheTypes.h"
#include "catapult/cache/CacheMixins.h"
#include "catapult/cache/ReadOnlySimpleCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Basic view on top of the hash cache.
	class BasicHashCacheView
			: public utils::MoveOnly
			, public SizeMixin<HashCacheTypes::BaseSetType>
			, public ContainsMixin<HashCacheTypes::BaseSetType, HashCacheDescriptor>
			, public SetIterationMixin<HashCacheTypes::BaseSetType, HashCacheDescriptor> {
	public:
		using ReadOnlyView = HashCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a view around \a hashes and \a options.
		explicit BasicHashCacheView(const HashCacheTypes::BaseSetType& hashes, const HashCacheTypes::Options& options)
				: SizeMixin<HashCacheTypes::BaseSetType>(hashes)
				, ContainsMixin<HashCacheTypes::BaseSetType, HashCacheDescriptor>(hashes)
				, SetIterationMixin<HashCacheTypes::BaseSetType, HashCacheDescriptor>(hashes)
				, m_retentionTime(options.RetentionTime)
		{}

	public:
		/// Gets the retention time for the cache.
		utils::TimeSpan retentionTime() const {
			return m_retentionTime;
		}

	private:
		utils::TimeSpan m_retentionTime;
	};

	/// View on top of the hash cache.
	class HashCacheView : public ReadOnlyViewSupplier<BasicHashCacheView> {
	public:
		/// Creates a view around \a hashes and \a options.
		explicit HashCacheView(const HashCacheTypes::BaseSetType& hashes, const HashCacheTypes::Options& options)
				: ReadOnlyViewSupplier(hashes, options)
		{}
	};
}}
