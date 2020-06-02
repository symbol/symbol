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
#include "NamespaceBaseSets.h"
#include "NamespaceCacheMixins.h"
#include "NamespaceCacheSerializers.h"
#include "ReadOnlyNamespaceCache.h"
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Mixins used by the namespace delta view.
	struct NamespaceCacheDeltaMixins {
	private:
		using PrimaryMixins = PatriciaTreeCacheMixins<NamespaceCacheTypes::PrimaryTypes::BaseSetDeltaType, NamespaceCacheDescriptor>;
		using FlatMapMixins = BasicCacheMixins<NamespaceCacheTypes::FlatMapTypes::BaseSetDeltaType, NamespaceCacheDescriptor>;

	public:
		using Size = PrimaryMixins::Size;
		using Contains = FlatMapMixins::Contains;
		using PatriciaTreeDelta = PrimaryMixins::PatriciaTreeDelta;
		using Touch = HeightBasedTouchMixin<
			typename NamespaceCacheTypes::PrimaryTypes::BaseSetDeltaType,
			typename NamespaceCacheTypes::HeightGroupingTypes::BaseSetDeltaType>;
		using DeltaElements = PrimaryMixins::DeltaElements;

		using NamespaceDeepSize = NamespaceDeepSizeMixin;
		using NamespaceLookup = NamespaceLookupMixin<
			NamespaceCacheTypes::PrimaryTypes::BaseSetDeltaType,
			NamespaceCacheTypes::FlatMapTypes::BaseSetDeltaType>;
	};

	/// Basic delta on top of the namespace cache.
	class BasicNamespaceCacheDelta
			: public utils::MoveOnly
			, public NamespaceCacheDeltaMixins::Size
			, public NamespaceCacheDeltaMixins::Contains
			, public NamespaceCacheDeltaMixins::PatriciaTreeDelta
			, public NamespaceCacheDeltaMixins::Touch
			, public NamespaceCacheDeltaMixins::DeltaElements
			, public NamespaceCacheDeltaMixins::NamespaceDeepSize
			, public NamespaceCacheDeltaMixins::NamespaceLookup {
	public:
		using ReadOnlyView = NamespaceCacheTypes::CacheReadOnlyType;
		using CollectedIds = std::unordered_set<NamespaceId, utils::BaseValueHasher<NamespaceId>>;

	public:
		/// Creates a delta around \a namespaceSets, \a options and \a namespaceSizes.
		BasicNamespaceCacheDelta(
				const NamespaceCacheTypes::BaseSetDeltaPointers& namespaceSets,
				const NamespaceCacheTypes::Options& options,
				const NamespaceSizes& namespaceSizes);

	public:
		/// Gets the grace period duration.
		BlockDuration gracePeriodDuration() const;

	public:
		/// Inserts the root namespace \a ns into the cache.
		void insert(const state::RootNamespace& ns);

		/// Inserts the namespace \a ns into the cache.
		void insert(const state::Namespace& ns);

		/// Sets the \a alias associated with namespace \a id.
		void setAlias(NamespaceId id, const state::NamespaceAlias& alias);

		/// Removes the namespace specified by its \a id from the cache.
		void remove(NamespaceId id);

		/// Prunes the cache at \a height.
		CollectedIds prune(Height height);

	private:
		void removeRoot(NamespaceId id);
		void removeChild(const state::Namespace& ns);

	private:
		NamespaceCacheTypes::PrimaryTypes::BaseSetDeltaPointerType m_pHistoryById;
		NamespaceCacheTypes::NamespaceCacheTypes::FlatMapTypes::BaseSetDeltaPointerType m_pNamespaceById;
		NamespaceCacheTypes::HeightGroupingTypes::BaseSetDeltaPointerType m_pRootNamespaceIdsByExpiryHeight;
		BlockDuration m_gracePeriodDuration;
	};

	/// Delta on top of the namespace cache.
	class NamespaceCacheDelta : public ReadOnlyViewSupplier<BasicNamespaceCacheDelta> {
	public:
		/// Creates a delta around \a namespaceSets, \a options and \a namespaceSizes.
		NamespaceCacheDelta(
				const NamespaceCacheTypes::BaseSetDeltaPointers& namespaceSets,
				const NamespaceCacheTypes::Options& options,
				const NamespaceSizes& namespaceSizes)
				: ReadOnlyViewSupplier(namespaceSets, options, namespaceSizes)
		{}
	};
}}
