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
#include "HashCacheTypes.h"
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlySimpleCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Mixins used by the hash cache delta.
	using HashCacheDeltaMixins = BasicCacheMixins<HashCacheTypes::PrimaryTypes::BaseSetDeltaType, HashCacheDescriptor>;

	/// Basic delta on top of the hash cache.
	class BasicHashCacheDelta
			: public utils::MoveOnly
			, public HashCacheDeltaMixins::Size
			, public HashCacheDeltaMixins::Contains
			, public HashCacheDeltaMixins::BasicInsertRemove
			, public HashCacheDeltaMixins::DeltaElements {
	public:
		using ReadOnlyView = HashCacheTypes::CacheReadOnlyType;
		using ValueType = HashCacheDescriptor::ValueType;

	public:
		/// Creates a delta around \a hashSets and \a options.
		BasicHashCacheDelta(const HashCacheTypes::BaseSetDeltaPointers& hashSets, const HashCacheTypes::Options& options);

	public:
		/// Gets the retention time for the cache.
		utils::TimeSpan retentionTime() const;

		/// Gets the pruning boundary that is used during commit.
		deltaset::PruningBoundary<ValueType> pruningBoundary() const;

	public:
		/// Removes all timestamped hashes that have timestamps prior to the given \a timestamp minus the retention time.
		void prune(Timestamp timestamp);

	private:
		HashCacheTypes::PrimaryTypes::BaseSetDeltaPointerType m_pOrderedDelta;
		utils::TimeSpan m_retentionTime;
		deltaset::PruningBoundary<ValueType> m_pruningBoundary;
	};

	/// Delta on top of the hash cache.
	class HashCacheDelta : public ReadOnlyViewSupplier<BasicHashCacheDelta> {
	public:
		/// Creates a delta around \a hashSets and \a options.
		HashCacheDelta(const HashCacheTypes::BaseSetDeltaPointers& hashSets, const HashCacheTypes::Options& options)
				: ReadOnlyViewSupplier(hashSets, options)
		{}
	};
}}
